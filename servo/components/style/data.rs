/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//! Per-node data used in style calculation.

use dom::{TElement, TRestyleDamage};
use properties::ComputedValues;
use restyle_hints::RestyleHint;
use selector_impl::{PseudoElement, RestyleDamage, Snapshot};
use std::collections::HashMap;
use std::hash::BuildHasherDefault;
use std::mem;
use std::ops::{Deref, DerefMut};
use std::sync::Arc;

type PseudoStylesInner = HashMap<PseudoElement, Arc<ComputedValues>,
                                 BuildHasherDefault<::fnv::FnvHasher>>;
#[derive(Clone, Debug)]
pub struct PseudoStyles(PseudoStylesInner);

impl PseudoStyles {
    pub fn empty() -> Self {
        PseudoStyles(HashMap::with_hasher(Default::default()))
    }
}

impl Deref for PseudoStyles {
    type Target = PseudoStylesInner;
    fn deref(&self) -> &Self::Target { &self.0 }
}

impl DerefMut for PseudoStyles {
    fn deref_mut(&mut self) -> &mut Self::Target { &mut self.0 }
}

/// The styles associated with a node, including the styles for any
/// pseudo-elements.
#[derive(Clone, Debug)]
pub struct ElementStyles {
    /// The results of CSS styling for this node.
    pub primary: Arc<ComputedValues>,

    /// The results of CSS styling for each pseudo-element (if any).
    pub pseudos: PseudoStyles,
}

impl ElementStyles {
    pub fn new(primary: Arc<ComputedValues>) -> Self {
        ElementStyles {
            primary: primary,
            pseudos: PseudoStyles::empty(),
        }
    }
}

#[derive(Debug)]
enum ElementDataStyles {
    /// The field has not been initialized.
    Uninitialized,

    /// The field holds the previous style of the node. If this is None, the
    /// node has not been previously styled.
    ///
    /// This is the input to the styling algorithm. It would ideally be
    /// immutable, but for now we need to mutate it a bit before styling to
    /// handle animations.
    ///
    /// Note that since ElementStyles contains an Arc, the null pointer
    /// optimization prevents the Option<> here from consuming an extra word.
    Previous(Option<ElementStyles>),

    /// The field holds the current, up-to-date style.
    ///
    /// This is the output of the styling algorithm.
    Current(ElementStyles),
}

impl ElementDataStyles {
    fn is_previous(&self) -> bool {
        use self::ElementDataStyles::*;
        match *self {
            Previous(_) => true,
            _ => false,
        }
    }
}

/// Enum to describe the different requirements that a restyle hint may impose
/// on its descendants.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum DescendantRestyleHint {
    /// This hint does not require any descendants to be restyled.
    Empty,
    /// This hint requires direct children to be restyled.
    Children,
    /// This hint requires all descendants to be restyled.
    Descendants,
}

impl DescendantRestyleHint {
    /// Propagates this descendant behavior to a child element.
    fn propagate(self) -> Self {
        use self::DescendantRestyleHint::*;
        if self == Descendants {
            Descendants
        } else {
            Empty
        }
    }

    fn union(self, other: Self) -> Self {
        use self::DescendantRestyleHint::*;
        if self == Descendants || other == Descendants {
            Descendants
        } else if self == Children || other == Children {
            Children
        } else {
            Empty
        }
    }
}

/// Restyle hint for storing on ElementData. We use a separate representation
/// to provide more type safety while propagating restyle hints down the tree.
#[derive(Clone, Debug)]
pub struct StoredRestyleHint {
    pub restyle_self: bool,
    pub descendants: DescendantRestyleHint,
}

impl StoredRestyleHint {
    /// Propagates this restyle hint to a child element.
    pub fn propagate(&self) -> Self {
        StoredRestyleHint {
            restyle_self: self.descendants == DescendantRestyleHint::Empty,
            descendants: self.descendants.propagate(),
        }
    }

    pub fn empty() -> Self {
        StoredRestyleHint {
            restyle_self: false,
            descendants: DescendantRestyleHint::Empty,
        }
    }

    pub fn is_empty(&self) -> bool {
        !self.restyle_self && self.descendants == DescendantRestyleHint::Empty
    }

    pub fn insert(&mut self, other: &Self) {
        self.restyle_self = self.restyle_self || other.restyle_self;
        self.descendants = self.descendants.union(other.descendants);
    }
}

impl Default for StoredRestyleHint {
    fn default() -> Self {
        StoredRestyleHint {
            restyle_self: false,
            descendants: DescendantRestyleHint::Empty,
        }
    }
}

impl From<RestyleHint> for StoredRestyleHint {
    fn from(hint: RestyleHint) -> Self {
        use restyle_hints::*;
        use self::DescendantRestyleHint::*;
        debug_assert!(hint.contains(RESTYLE_LATER_SIBLINGS), "Caller should apply sibling hints");
        StoredRestyleHint {
            restyle_self: hint.contains(RESTYLE_SELF),
            descendants: if hint.contains(RESTYLE_DESCENDANTS) { Descendants } else { Empty },
        }
    }
}

/// Transient data used by the restyle algorithm. This structure is instantiated
/// either before or during restyle traversal, and is cleared at the end of node
/// processing.
#[derive(Debug)]
pub struct RestyleData {
    pub hint: StoredRestyleHint,
    pub damage: RestyleDamage,
    pub snapshot: Option<Snapshot>,
}

impl RestyleData {
    fn new() -> Self {
        RestyleData {
            hint: StoredRestyleHint::default(),
            damage: RestyleDamage::empty(),
            snapshot: None,
        }
    }

    pub fn ensure_snapshot<E: TElement>(&mut self, element: E) -> &mut Snapshot {
        if self.snapshot.is_none() {
            self.snapshot = Some(element.create_snapshot());
        }

        self.snapshot.as_mut().unwrap()
    }
}

/// Style system data associated with a node.
///
/// In Gecko, this hangs directly off a node, but is dropped when the frame takes
/// ownership of the computed style data.
///
/// In Servo, this is embedded inside of layout data, which itself hangs directly
/// off the node. Servo does not currently implement ownership transfer of the
/// computed style data to the frame.
///
/// In both cases, it is wrapped inside an AtomicRefCell to ensure thread
/// safety.
#[derive(Debug)]
pub struct ElementData {
    styles: ElementDataStyles,
    pub restyle_data: Option<RestyleData>,
}

impl ElementData {
    pub fn new() -> Self {
        ElementData {
            styles: ElementDataStyles::Uninitialized,
            restyle_data: None,
        }
    }

    pub fn has_current_styles(&self) -> bool {
        match self.styles {
            ElementDataStyles::Current(_) => true,
            _ => false,
        }
    }

    pub fn get_current_styles(&self) -> Option<&ElementStyles> {
        match self.styles {
            ElementDataStyles::Current(ref s) => Some(s),
            _ => None,
        }
    }

    pub fn current_styles(&self) -> &ElementStyles {
        self.get_current_styles().expect("Calling current_styles before or during styling")
    }

    // Servo does lazy pseudo computation in layout and needs mutable access
    // to the current styles
    #[cfg(not(feature = "gecko"))]
    pub fn current_pseudos_mut(&mut self) -> &mut PseudoStyles {
        match self.styles {
            ElementDataStyles::Current(ref mut s) => &mut s.pseudos,
            _ => panic!("Calling current_pseudos_mut before or during styling"),
        }
    }

    pub fn previous_styles(&self) -> Option<&ElementStyles> {
        match self.styles {
            ElementDataStyles::Previous(ref s) => s.as_ref(),
            _ => panic!("Calling previous_styles without having gathered it"),
        }
    }

    pub fn previous_styles_mut(&mut self) -> Option<&mut ElementStyles> {
        match self.styles {
            ElementDataStyles::Previous(ref mut s) => s.as_mut(),
            _ => panic!("Calling previous_styles without having gathered it"),
        }
    }

    pub fn gather_previous_styles<F>(&mut self, f: F)
        where F: FnOnce() -> Option<ElementStyles>
    {
        use self::ElementDataStyles::*;
        self.styles = match mem::replace(&mut self.styles, Uninitialized) {
            Uninitialized => Previous(f()),
            Current(x) => Previous(Some(x)),
            Previous(x) => Previous(x),
        };
    }

    pub fn ensure_restyle_data(&mut self) -> &mut RestyleData {
        println!("Before: {:?}\n", self.restyle_data);
        if self.restyle_data.is_none() {
            self.restyle_data = Some(RestyleData::new());
            println!("After: {:?}\n", self.restyle_data);
        }
        let result = self.restyle_data.as_mut().unwrap();
        result
    }

    pub fn finish_styling(&mut self, styles: ElementStyles) {
        debug_assert!(self.styles.is_previous());
        self.styles = ElementDataStyles::Current(styles);
    }
}
