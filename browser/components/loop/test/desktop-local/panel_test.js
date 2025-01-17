/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

describe("loop.panel", function() {
  "use strict";

  var expect = chai.expect;
  var TestUtils = React.addons.TestUtils;
  var sharedActions = loop.shared.actions;
  var sharedUtils = loop.shared.utils;

  var sandbox, notifications;
  var fakeXHR, fakeWindow, fakeMozLoop, fakeEvent;
  var requests = [];
  var roomData, roomData2, roomList, roomName;
  var mozL10nGetSpy;

  beforeEach(function() {
    sandbox = sinon.sandbox.create();
    fakeXHR = sandbox.useFakeXMLHttpRequest();
    requests = [];
    // https://github.com/cjohansen/Sinon.JS/issues/393
    fakeXHR.xhr.onCreate = function(xhr) {
      requests.push(xhr);
    };

    fakeEvent = {
      preventDefault: sandbox.stub(),
      stopPropagation: sandbox.stub(),
      pageY: 42
    };

    fakeWindow = {
      close: sandbox.stub(),
      addEventListener: function() {},
      document: { addEventListener: function() {} },
      setTimeout: function(callback) { callback(); }
    };
    loop.shared.mixins.setRootObject(fakeWindow);

    notifications = new loop.shared.models.NotificationCollection();

    fakeMozLoop = navigator.mozLoop = {
      doNotDisturb: true,
      fxAEnabled: true,
      getStrings: function() {
        return JSON.stringify({ textContent: "fakeText" });
      },
      get locale() {
        return "en-US";
      },
      setLoopPref: sandbox.stub(),
      getLoopPref: function(prefName) {
        return "unseen";
      },
      getPluralForm: function() {
        return "fakeText";
      },
      rooms: {
        getAll: function(version, callback) {
          callback(null, []);
        },
        on: sandbox.stub()
      },
      confirm: sandbox.stub(),
      hasEncryptionKey: true,
      hangupAllChatWindows: function() {},
      logInToFxA: sandbox.stub(),
      logOutFromFxA: sandbox.stub(),
      notifyUITour: sandbox.stub(),
      openURL: sandbox.stub(),
      getSelectedTabMetadata: sandbox.stub(),
      userProfile: null
    };

    roomName = "First Room Name";
    roomData = {
      roomToken: "QzBbvGmIZWU",
      roomUrl: "http://sample/QzBbvGmIZWU",
      decryptedContext: {
        roomName: roomName
      },
      maxSize: 2,
        participants: [{
        displayName: "Alexis",
          account: "alexis@example.com",
          roomConnectionId: "2a1787a6-4a73-43b5-ae3e-906ec1e763cb"
      }, {
        displayName: "Adam",
        roomConnectionId: "781f012b-f1ea-4ce1-9105-7cfc36fb4ec7"
    }],
      ctime: 1405517418
    };

    roomData2 = {
      roomToken: "QzBbvlmIZWU",
      roomUrl: "http://sample/QzBbvlmIZWU",
      decryptedContext: {
        roomName: "Second Room Name"
      },
      maxSize: 2,
        participants: [{
        displayName: "Bill",
          account: "bill@example.com",
          roomConnectionId: "2a1737a6-4a73-43b5-ae3e-906ec1e763cb"
      }, {
          displayName: "Bob",
            roomConnectionId: "781f212b-f1ea-4ce1-9105-7cfc36fb4ec7"
        }],
      ctime: 1405517417
    };

    roomList = [new loop.store.Room(roomData), new loop.store.Room(roomData2)];

    document.mozL10n.initialize(navigator.mozLoop);
    sandbox.stub(document.mozL10n, "get").returns("Fake title");
  });

  afterEach(function() {
    delete navigator.mozLoop;
    loop.shared.mixins.setRootObject(window);
    sandbox.restore();
  });

  describe("#init", function() {
    beforeEach(function() {
      sandbox.stub(React, "render");
      sandbox.stub(document.mozL10n, "initialize");
    });

    it("should initalize L10n", function() {
      loop.panel.init();

      sinon.assert.calledOnce(document.mozL10n.initialize);
      sinon.assert.calledWithExactly(document.mozL10n.initialize,
        navigator.mozLoop);
    });

    it("should render the panel view", function() {
      loop.panel.init();

      sinon.assert.calledOnce(React.render);
      sinon.assert.calledWith(React.render,
        sinon.match(function(value) {
          return TestUtils.isCompositeComponentElement(value,
            loop.panel.PanelView);
      }));
    });

    it("should dispatch an loopPanelInitialized", function(done) {
      function listener() {
        done();
      }

      window.addEventListener("loopPanelInitialized", listener);

      loop.panel.init();

      window.removeEventListener("loopPanelInitialized", listener);
    });
  });

  describe("loop.panel.PanelView", function() {
    var fakeClient, dispatcher, roomStore, callUrlData;

    beforeEach(function() {
      callUrlData = {
        callUrl: "http://call.invalid/",
        expiresAt: 1000
      };

      fakeClient = {
        requestCallUrl: function(_, cb) {
          cb(null, callUrlData);
        }
      };

      dispatcher = new loop.Dispatcher();
      roomStore = new loop.store.RoomStore(dispatcher, {
        mozLoop: fakeMozLoop
      });
    });

    function createTestPanelView() {
      return TestUtils.renderIntoDocument(
        React.createElement(loop.panel.PanelView, {
          notifications: notifications,
          client: fakeClient,
          showTabButtons: true,
          mozLoop: fakeMozLoop,
          dispatcher: dispatcher,
          roomStore: roomStore
        }));
    }

    it("should hide the account entry when FxA is not enabled", function() {
      navigator.mozLoop.userProfile = { email: "test@example.com" };
      navigator.mozLoop.fxAEnabled = false;

      var view = TestUtils.renderIntoDocument(
        React.createElement(loop.panel.SettingsDropdown, {
          mozLoop: fakeMozLoop
        }));

      expect(view.getDOMNode().querySelectorAll(".icon-account"))
        .to.have.length.of(0);
    });

    describe("AccountLink", function() {
      beforeEach(function() {
        navigator.mozLoop.calls = { clearCallInProgress: function() {} };
      });

      afterEach(function() {
        delete navigator.mozLoop.logInToFxA;
        delete navigator.mozLoop.calls;
        navigator.mozLoop.fxAEnabled = true;
      });

      it("should NOT show the context menu on right click", function() {
        var prevent = sandbox.stub();
        var view = createTestPanelView();
        TestUtils.Simulate.contextMenu(
          view.getDOMNode(),
          { preventDefault: prevent }
        );
        sinon.assert.calledOnce(prevent);
      });

      it("should trigger the FxA sign in/up process when clicking the link",
        function() {
          navigator.mozLoop.logInToFxA = sandbox.stub();

          var view = createTestPanelView();

          TestUtils.Simulate.click(
            view.getDOMNode().querySelector(".signin-link > a"));

          sinon.assert.calledOnce(navigator.mozLoop.logInToFxA);
        });

      it("should close the panel after clicking the link",
        function() {
          navigator.mozLoop.loggedInToFxA = false;
          navigator.mozLoop.logInToFxA = sandbox.stub();

          var view = createTestPanelView();

          TestUtils.Simulate.click(
            view.getDOMNode().querySelector(".signin-link > a"));

          sinon.assert.calledOnce(fakeWindow.close);
        });

      it("should be hidden if FxA is not enabled",
        function() {
          navigator.mozLoop.fxAEnabled = false;
          var view = TestUtils.renderIntoDocument(
            React.createElement(loop.panel.AccountLink, {
              fxAEnabled: false,
              userProfile: null
            }));
          expect(view.getDOMNode()).to.be.null;
      });

      it("should add ellipsis to text over 24chars", function() {
        navigator.mozLoop.userProfile = {
          email: "reallyreallylongtext@example.com"
        };
        var view = createTestPanelView();
        var node = view.getDOMNode().querySelector(".user-identity");

        expect(node.textContent).to.eql("reallyreallylongtext@exa…");
      });

      it("should warn when user profile is different from {} or null",
         function() {
          var warnstub = sandbox.stub(console, "warn");

          var view = TestUtils.renderIntoDocument(React.createElement(
            loop.panel.AccountLink, {
              fxAEnabled: false,
              userProfile: []
            }
          ));

          sinon.assert.calledOnce(warnstub);
          sinon.assert.calledWithMatch(warnstub, "Required prop `userProfile` was not correctly specified in `AccountLink`.");
      });

      it("should not warn when user profile is an object",
         function() {
          var warnstub = sandbox.stub(console, "warn");

          var view = TestUtils.renderIntoDocument(React.createElement(
            loop.panel.AccountLink, {
              fxAEnabled: false,
              userProfile: {}
            }
          ));

          sinon.assert.notCalled(warnstub);
      });
    });

    describe("SettingsDropdown", function() {
      function mountTestComponent() {
        return TestUtils.renderIntoDocument(
          React.createElement(loop.panel.SettingsDropdown, {
            mozLoop: fakeMozLoop
          }));
      }

      beforeEach(function() {
        navigator.mozLoop.logInToFxA = sandbox.stub();
        navigator.mozLoop.logOutFromFxA = sandbox.stub();
        navigator.mozLoop.openFxASettings = sandbox.stub();
      });

      afterEach(function() {
        navigator.mozLoop.fxAEnabled = true;
      });

      describe("UserLoggedOut", function() {
        beforeEach(function() {
          fakeMozLoop.userProfile = null;
        });

        it("should show a signin entry when user is not authenticated",
           function() {
             var view = mountTestComponent();

             expect(view.getDOMNode().querySelectorAll(".entry-settings-signout"))
               .to.have.length.of(0);
             expect(view.getDOMNode().querySelectorAll(".entry-settings-signin"))
               .to.have.length.of(1);
           });

        it("should hide any account entry when user is not authenticated",
           function() {
             var view = mountTestComponent();

             expect(view.getDOMNode().querySelectorAll(".icon-account"))
               .to.have.length.of(0);
           });

        it("should sign in the user on click when unauthenticated", function() {
          navigator.mozLoop.loggedInToFxA = false;
          var view = mountTestComponent();

          TestUtils.Simulate.click(view.getDOMNode()
                                     .querySelector(".entry-settings-signin"));

          sinon.assert.calledOnce(navigator.mozLoop.logInToFxA);
        });
      });

      it("should show a signout entry when user is authenticated", function() {
        navigator.mozLoop.userProfile = { email: "test@example.com" };

        var view = mountTestComponent();

        sinon.assert.calledWithExactly(document.mozL10n.get,
                                       "settings_menu_item_signout");
        sinon.assert.neverCalledWith(document.mozL10n.get,
                                     "settings_menu_item_signin");
      });

      it("should show an account entry when user is authenticated", function() {
        navigator.mozLoop.userProfile = { email: "test@example.com" };

        var view = mountTestComponent();

        sinon.assert.calledWithExactly(document.mozL10n.get,
                                       "settings_menu_item_settings");
      });

      it("should open the FxA settings when the account entry is clicked",
         function() {
           navigator.mozLoop.userProfile = { email: "test@example.com" };

           var view = mountTestComponent();

           TestUtils.Simulate.click(view.getDOMNode()
                                      .querySelector(".entry-settings-account"));

           sinon.assert.calledOnce(navigator.mozLoop.openFxASettings);
         });

      it("should sign out the user on click when authenticated", function() {
        navigator.mozLoop.userProfile = { email: "test@example.com" };
        var view = mountTestComponent();

        TestUtils.Simulate.click(view.getDOMNode()
                                   .querySelector(".entry-settings-signout"));

        sinon.assert.calledOnce(navigator.mozLoop.logOutFromFxA);
      });

      describe("Toggle Notifications", function() {
        var view;

        beforeEach(function() {
          view = mountTestComponent();
        });

        it("should toggle mozLoop.doNotDisturb to false", function() {
          navigator.mozLoop.doNotDisturb = true;
          var toggleNotificationsMenuOption = view.getDOMNode()
                                                .querySelector(".entry-settings-notifications");

          TestUtils.Simulate.click(toggleNotificationsMenuOption);

          expect(navigator.mozLoop.doNotDisturb).eql(false);
        });

        it("should toggle mozLoop.doNotDisturb to true", function() {
          navigator.mozLoop.doNotDisturb = false;
          var toggleNotificationsMenuOption = view.getDOMNode()
                                                .querySelector(".entry-settings-notifications");

          TestUtils.Simulate.click(toggleNotificationsMenuOption);

          expect(navigator.mozLoop.doNotDisturb).eql(true);
        });

        it("should close dropdown menu", function() {
          navigator.mozLoop.doNotDisturb = true;
          var toggleNotificationsMenuOption = view.getDOMNode()
                                                .querySelector(".entry-settings-notifications");

          view.setState({ showMenu: true });

          TestUtils.Simulate.click(toggleNotificationsMenuOption);

          expect(view.state.showMenu).eql(false);
        });
      });
    });

    describe("Help", function() {
      var view, supportUrl;

      function mountTestComponent() {
        return TestUtils.renderIntoDocument(
          React.createElement(loop.panel.SettingsDropdown, {
            mozLoop: fakeMozLoop
          }));
      }

      beforeEach(function() {
        supportUrl = "https://example.com";
        navigator.mozLoop.getLoopPref = function(pref) {
          if (pref === "support_url") {
            return supportUrl;
          }

          return "unseen";
        };
      });

      it("should open a tab to the support page", function() {
        view = mountTestComponent();

        TestUtils.Simulate
          .click(view.getDOMNode().querySelector(".entry-settings-help"));

        sinon.assert.calledOnce(fakeMozLoop.openURL);
        sinon.assert.calledWithExactly(fakeMozLoop.openURL, supportUrl);
      });

      it("should close the panel", function() {
        view = mountTestComponent();

        TestUtils.Simulate
          .click(view.getDOMNode().querySelector(".entry-settings-help"));

        sinon.assert.calledOnce(fakeWindow.close);
      });
    });

    describe("Submit feedback", function() {
      var view, feedbackUrl;

      function mountTestComponent() {
        return TestUtils.renderIntoDocument(
          React.createElement(loop.panel.SettingsDropdown, {
            mozLoop: fakeMozLoop
          }));
      }

      beforeEach(function() {
        feedbackUrl = "https://example.com";
        fakeMozLoop.getLoopPref = function(pref) {
          if (pref === "feedback.formURL") {
            return feedbackUrl;
          }

          return "unseen";
        };
      });

      it("should open a tab to the feedback page", function() {
        view = mountTestComponent();

        TestUtils.Simulate
          .click(view.getDOMNode().querySelector(".entry-settings-feedback"));

        sinon.assert.calledOnce(fakeMozLoop.openURL);
        sinon.assert.calledWithExactly(fakeMozLoop.openURL, feedbackUrl);
      });

      it("should close the panel", function() {
        view = mountTestComponent();

        TestUtils.Simulate
          .click(view.getDOMNode().querySelector(".entry-settings-feedback"));

        sinon.assert.calledOnce(fakeWindow.close);
      });
    });

    describe("#render", function() {
      it("should not render a ToSView when gettingStarted.seen is true", function() {
        navigator.mozLoop.getLoopPref = function() {
          return true;
        };
        var view = createTestPanelView();

        expect(function() {
          TestUtils.findRenderedComponentWithType(view, loop.panel.ToSView);
        }).to.Throw(/not find/);
      });

      it("should not render a ToSView when gettingStarted.seen is false", function() {
        navigator.mozLoop.getLoopPref = function() {
          return false;
        };
        var view = createTestPanelView();

        expect(function() {
          TestUtils.findRenderedComponentWithType(view, loop.panel.ToSView);
        }).to.not.Throw();
      });

      it("should render a GettingStarted view", function() {
        navigator.mozLoop.getLoopPref = function(pref) {
          return false;
        };
        var view = createTestPanelView();

        TestUtils.findRenderedComponentWithType(view, loop.panel.GettingStartedView);
      });

      it("should not render a GettingStartedView when the view has been seen", function() {
        navigator.mozLoop.getLoopPref = function() {
          return true;
        };
        var view = createTestPanelView();

        try {
          TestUtils.findRenderedComponentWithType(view, loop.panel.GettingStartedView);
          sinon.assert.fail("Should not find the GettingStartedView if it has been seen");
        } catch (ex) {
          // Do nothing
        }
      });

      it("should render a SignInRequestView when mozLoop.hasEncryptionKey is false", function() {
        fakeMozLoop.hasEncryptionKey = false;

        var view = createTestPanelView();

        TestUtils.findRenderedComponentWithType(view, loop.panel.SignInRequestView);
      });

      it("should render a SignInRequestView when mozLoop.hasEncryptionKey is true", function() {
        var view = createTestPanelView();

        try {
          TestUtils.findRenderedComponentWithType(view, loop.panel.SignInRequestView);
          sinon.assert.fail("Should not find the GettingStartedView if it has been seen");
        } catch (ex) {
          // Do nothing
        }
      });
    });
  });

  describe("loop.panel.RoomEntry", function() {
    var dispatcher;

    beforeEach(function() {
      dispatcher = new loop.Dispatcher();
    });

    function mountRoomEntry(props) {
      props = _.extend({
        dispatcher: dispatcher,
        mozLoop: fakeMozLoop
      }, props);
      return TestUtils.renderIntoDocument(
        React.createElement(loop.panel.RoomEntry, props));
    }

    describe("handleClick", function() {
      var view;

      beforeEach(function() {
        // Stub to prevent warnings due to stores not being set up to handle
        // the actions we are triggering.
        sandbox.stub(dispatcher, "dispatch");

        view = mountRoomEntry({
          isOpenedRoom: false,
          room: new loop.store.Room(roomData)
        });
      });

      // XXX Current version of React cannot use TestUtils.Simulate, please
      // enable when we upgrade.
      it.skip("should close the menu when you move out the cursor", function() {
        expect(view.refs.contextActions.state.showMenu).to.eql(false);
      });

      it("should set eventPosY when handleClick is called", function() {
        view.handleClick(fakeEvent);

        expect(view.state.eventPosY).to.eql(fakeEvent.pageY);
      });

      it("toggle state.showMenu when handleClick is called", function() {
        var prevState = view.state.showMenu;
        view.handleClick(fakeEvent);

        expect(view.state.showMenu).to.eql(!prevState);
      });

      it("should toggle the menu when the button is clicked", function() {
        var prevState = view.state.showMenu;
        var node = view.refs.contextActions.refs["menu-button"].getDOMNode();

        TestUtils.Simulate.click(node, fakeEvent);

        expect(view.state.showMenu).to.eql(!prevState);
      });
    });

    describe("Copy button", function() {
      var roomEntry;

      beforeEach(function() {
        // Stub to prevent warnings where no stores are set up to handle the
        // actions we are testing.
        sandbox.stub(dispatcher, "dispatch");

        roomEntry = mountRoomEntry({
          deleteRoom: sandbox.stub(),
          isOpenedRoom: false,
          room: new loop.store.Room(roomData)
        });
      });

      it("should render context actions button", function() {
        expect(roomEntry.refs.contextActions).to.not.eql(null);
      });

      describe("OpenRoom", function() {
        it("should dispatch an OpenRoom action when button is clicked", function() {
          TestUtils.Simulate.click(roomEntry.refs.roomEntry.getDOMNode());

          sinon.assert.calledOnce(dispatcher.dispatch);
          sinon.assert.calledWithExactly(dispatcher.dispatch,
            new sharedActions.OpenRoom({ roomToken: roomData.roomToken }));
        });

        it("should dispatch an OpenRoom action when callback is called", function() {
          roomEntry.handleClickEntry(fakeEvent);

          sinon.assert.calledOnce(dispatcher.dispatch);
          sinon.assert.calledWithExactly(dispatcher.dispatch,
            new sharedActions.OpenRoom({ roomToken: roomData.roomToken }));
        });

        it("should call window.close", function() {
          roomEntry.handleClickEntry(fakeEvent);

          sinon.assert.calledOnce(fakeWindow.close);
        });

        it("should not dispatch an OpenRoom action when button is clicked if room is already opened", function() {
          roomEntry = mountRoomEntry({
            deleteRoom: sandbox.stub(),
            isOpenedRoom: true,
            room: new loop.store.Room(roomData)
          });

          TestUtils.Simulate.click(roomEntry.refs.roomEntry.getDOMNode());

          sinon.assert.notCalled(dispatcher.dispatch);
        });
      });
    });

    describe("Context Indicator", function() {
      var roomEntry;

      function mountEntryForContext() {
        return mountRoomEntry({
          isOpenedRoom: false,
          room: new loop.store.Room(roomData)
        });
      }

      it("should display a default context indicator if the room doesn't have any", function() {
        roomEntry = mountEntryForContext();

        expect(roomEntry.getDOMNode().querySelector(".room-entry-context-item")).not.eql(null);
      });

      it("should a context indicator if the room specifies context", function() {
        roomData.decryptedContext.urls = [{
          description: "invalid entry",
          location: "http://invalid",
          thumbnail: "data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw=="
        }];

        roomEntry = mountEntryForContext();

        expect(roomEntry.getDOMNode().querySelector(".room-entry-context-item")).not.eql(null);
      });

      it("should call mozLoop.openURL to open a new url", function() {
        roomData.decryptedContext.urls = [{
          description: "invalid entry",
          location: "http://invalid/",
          thumbnail: "data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw=="
        }];

        roomEntry = mountEntryForContext();

        TestUtils.Simulate.click(roomEntry.getDOMNode().querySelector("a"));

        sinon.assert.calledOnce(fakeMozLoop.openURL);
        sinon.assert.calledWithExactly(fakeMozLoop.openURL, "http://invalid/");
      });

      it("should call close the panel after opening a url", function() {
        roomData.decryptedContext.urls = [{
          description: "invalid entry",
          location: "http://invalid/",
          thumbnail: "data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw=="
        }];

        roomEntry = mountEntryForContext();

        TestUtils.Simulate.click(roomEntry.getDOMNode().querySelector("a"));

        sinon.assert.calledOnce(fakeWindow.close);
      });
    });

    describe("Room Entry click", function() {
      var roomEntry, roomEntryNode;

      beforeEach(function() {
        sandbox.stub(dispatcher, "dispatch");

        roomEntry = mountRoomEntry({
          dispatcher: dispatcher,
          isOpenedRoom: false,
          room: new loop.store.Room(roomData)
        });
        roomEntryNode = roomEntry.getDOMNode();
      });

    });

    describe("Room name updated", function() {
      it("should update room name", function() {
        var roomEntry = mountRoomEntry({
          dispatcher: dispatcher,
          isOpenedRoom: false,
          room: new loop.store.Room(roomData)
        });
        var updatedRoom = new loop.store.Room(_.extend({}, roomData, {
          decryptedContext: {
            roomName: "New room name"
          },
          ctime: new Date().getTime()
        }));

        roomEntry.setProps({ room: updatedRoom });

        expect(
          roomEntry.getDOMNode().textContent)
        .eql("New room name");
      });
    });

    describe("Room name priority", function() {
      var roomEntry;
      beforeEach(function() {
        roomEntry = mountRoomEntry({
          dispatcher: dispatcher,
          isOpenedRoom: false,
          room: new loop.store.Room(roomData)
        });
      });

      function setDecryptedContext(newDecryptedContext) {
        return new loop.store.Room(_.extend({}, roomData, {
          decryptedContext: newDecryptedContext,
          ctime: new Date().getTime()
        }));
      }

      it("should use room name by default", function() {
        var updatedRoom = setDecryptedContext({
          roomName: "Room name",
          urls: [
            {
              description: "Website title",
              location: "https://fakeurl.com"
            }
          ]
        });

        roomEntry.setProps({ room: updatedRoom });

        expect(roomEntry.getDOMNode().textContent).eql("Room name");
      });

      it("should use context title when there's no room title", function() {
        var updatedRoom = setDecryptedContext({
          urls: [
            {
              description: "Website title",
              location: "https://fakeurl.com"
            }
          ]
        });

        roomEntry.setProps({ room: updatedRoom });

        expect(roomEntry.getDOMNode().textContent).eql("Website title");
      });

      it("should use website url when there's no room title nor website", function() {
        var updatedRoom = setDecryptedContext({
          urls: [
            {
              location: "https://fakeurl.com"
            }
          ]
        });

        roomEntry.setProps({ room: updatedRoom });

        expect(roomEntry.getDOMNode().textContent).eql("https://fakeurl.com");
      });
    });
  });

  describe("loop.panel.RoomList", function() {
    var roomStore, dispatcher, fakeEmail, dispatch;

    beforeEach(function() {
      fakeEmail = "fakeEmail@example.com";
      dispatcher = new loop.Dispatcher();
      roomStore = new loop.store.RoomStore(dispatcher, {
        mozLoop: navigator.mozLoop
      });
      roomStore.setStoreState({
        openedRoom: null,
        pendingCreation: false,
        pendingInitialRetrieval: false,
        rooms: [],
        error: undefined
      });

      dispatch = sandbox.stub(dispatcher, "dispatch");
    });

    function createTestComponent() {
      return TestUtils.renderIntoDocument(
        React.createElement(loop.panel.RoomList, {
          store: roomStore,
          dispatcher: dispatcher,
          userDisplayName: fakeEmail,
          mozLoop: fakeMozLoop,
          userProfile: null
        }));
    }

    it("should dispatch a GetAllRooms action on mount", function() {
      createTestComponent();

      sinon.assert.calledOnce(dispatch);
      sinon.assert.calledWithExactly(dispatch, new sharedActions.GetAllRooms());
    });

    it("should close the panel once a room is created and there is no error", function() {
      var view = createTestComponent();

      roomStore.setStoreState({ pendingCreation: true });

      sinon.assert.notCalled(fakeWindow.close);

      roomStore.setStoreState({ pendingCreation: false });

      sinon.assert.calledOnce(fakeWindow.close);
    });

    it("should render the no rooms view when no rooms available", function() {
      var view = createTestComponent();
      var node = view.getDOMNode();

      expect(node.querySelectorAll(".room-list-empty").length).to.eql(1);
    });

    it("should call mozL10n.get for room empty strings", function() {
      var view = createTestComponent();

      sinon.assert.calledWithExactly(document.mozL10n.get,
                                     "no_conversations_message_heading2");
      sinon.assert.calledWithExactly(document.mozL10n.get,
                                     "no_conversations_start_message2");
    });

    it("should display a loading animation when rooms are pending", function() {
      var view = createTestComponent();
      roomStore.setStoreState({ pendingInitialRetrieval: true });

      expect(view.getDOMNode().querySelectorAll(".room-list-loading").length).to.eql(1);
    });

    it("should show multiple rooms in list with no opened room", function() {
      roomStore.setStoreState({ rooms: roomList });

      var view = createTestComponent();

      var node = view.getDOMNode();
      expect(node.querySelectorAll(".room-opened").length).to.eql(0);
      expect(node.querySelectorAll(".room-entry").length).to.eql(2);
    });

    it("should only show the opened room you're in when you're in a room", function() {
      roomStore.setStoreState({ rooms: roomList, openedRoom: roomList[0].roomToken });

      var view = createTestComponent();

      var node = view.getDOMNode();
      expect(node.querySelectorAll(".room-opened").length).to.eql(1);
      expect(node.querySelectorAll(".room-entry").length).to.eql(1);
      expect(node.querySelectorAll(".room-opened h2")[0].textContent).to.equal(roomName);
    });

    it("should show Page Title as Room Name if a Room Name is not given", function() {
      var urlsRoomData = {
        roomToken: "QzBbvGmIZWU",
        roomUrl: "http://sample/QzBbvGmIZWU",
        decryptedContext: {
          urls: [{
            description: "Page Title",
            location: "http://example.com"
          }]
        },
        maxSize: 2,
        participants: [{
          displayName: "Alexis",
          account: "alexis@example.com",
          roomConnectionId: "2a1787a6-4a73-43b5-ae3e-906ec1e763cb"
        }, {
          displayName: "Adam",
          roomConnectionId: "781f012b-f1ea-4ce1-9105-7cfc36fb4ec7"
        }],
        ctime: 1405517418
      };
      roomStore.setStoreState({ rooms: [new loop.store.Room(urlsRoomData)] });

      var view = createTestComponent();

      var node = view.getDOMNode();
      expect(node.querySelector(".room-entry h2").textContent).to.equal("Page Title");
    });

    it("should show Page URL as Room Name if a Room Name and Page Title are not available", function() {
      var urlsRoomData = {
        roomToken: "QzBbvGmIZWU",
        roomUrl: "http://sample/QzBbvGmIZWU",
        decryptedContext: {
          urls: [{
            description: "",
            location: "http://example.com"
          }]
        },
        maxSize: 2,
        participants: [{
          displayName: "Alexis",
          account: "alexis@example.com",
          roomConnectionId: "2a1787a6-4a73-43b5-ae3e-906ec1e763cb"
        }, {
          displayName: "Adam",
          roomConnectionId: "781f012b-f1ea-4ce1-9105-7cfc36fb4ec7"
        }],
        ctime: 1405517418
      };
      roomStore.setStoreState({ rooms: [new loop.store.Room(urlsRoomData)] });

      var view = createTestComponent();

      var node = view.getDOMNode();
      expect(node.querySelector(".room-entry h2").textContent).to.equal("http://example.com");
    });

    it("should show Fallback Title as Room Name if a Room Name,Page Title and Page Url are not available", function() {
      var urlsRoomData = {
        roomToken: "QzBbvGmIZWU",
        roomUrl: "http://sample/QzBbvGmIZWU",
        decryptedContext: {
          urls: [{
            description: "",
            location: ""
          }]
        },
        maxSize: 2,
        participants: [{
          displayName: "Alexis",
          account: "alexis@example.com",
          roomConnectionId: "2a1787a6-4a73-43b5-ae3e-906ec1e763cb"
        }, {
          displayName: "Adam",
          roomConnectionId: "781f012b-f1ea-4ce1-9105-7cfc36fb4ec7"
        }],
        ctime: 1405517418
      };
      roomStore.setStoreState({ rooms: [new loop.store.Room(urlsRoomData)] });

      var view = createTestComponent();

      var node = view.getDOMNode();
      expect(node.querySelector(".room-entry h2").textContent).to.equal("Fake title");
    });
  });

  describe("loop.panel.NewRoomView", function() {
    var roomStore, dispatcher, fakeEmail, dispatch;

    beforeEach(function() {
      fakeEmail = "fakeEmail@example.com";
      dispatcher = new loop.Dispatcher();
      roomStore = new loop.store.RoomStore(dispatcher, {
        mozLoop: navigator.mozLoop
      });
      roomStore.setStoreState({
        pendingCreation: false,
        pendingInitialRetrieval: false,
        rooms: [],
        error: undefined
      });
      dispatch = sandbox.stub(dispatcher, "dispatch");
    });

    function createTestComponent(extraProps) {
      return TestUtils.renderIntoDocument(
        React.createElement(loop.panel.NewRoomView, _.extend({
          dispatcher: dispatcher,
          mozLoop: fakeMozLoop,
          userDisplayName: fakeEmail
        }, extraProps)));
    }

    it("should dispatch a CreateRoom action with context when clicking on the " +
       "Start a conversation button", function() {
      fakeMozLoop.userProfile = { email: fakeEmail };
      var favicon = "data:image/x-icon;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==";
      fakeMozLoop.getSelectedTabMetadata = function(callback) {
        callback({
          url: "http://invalid.com",
          description: "fakeSite",
          favicon: favicon,
          previews: ["fakeimage.png"]
        });
      };

      var view = createTestComponent({
        inRoom: false,
        pendingOperation: false
      });

      // Simulate being visible
      view.onDocumentVisible();

      var node = view.getDOMNode();

      TestUtils.Simulate.click(node.querySelector(".new-room-button"));

      sinon.assert.calledWith(dispatch, new sharedActions.CreateRoom({
        urls: [{
          location: "http://invalid.com",
          description: "fakeSite",
          thumbnail: favicon
        }]
      }));
    });

    it("should disable the create button when pendingOperation is true",
      function() {
        var view = createTestComponent({
          inRoom: false,
          pendingOperation: true
        });

        var buttonNode = view.getDOMNode().querySelector(".new-room-button[disabled]");
        expect(buttonNode).to.not.equal(null);
      });

    it("should not have a create button when inRoom is true", function() {
      var view = createTestComponent({
        inRoom: true,
        pendingOperation: false
      });

      var buttonNode = view.getDOMNode().querySelector(".new-room-button");
      expect(buttonNode).to.equal(null);
    });

    it("should have a stop sharing button when inRoom is true", function() {
      var view = createTestComponent({
        inRoom: true,
        pendingOperation: false
      });

      var buttonNode = view.getDOMNode().querySelector(".stop-sharing-button");
      expect(buttonNode).to.not.equal(null);
    });

    it("should hangup any window when stop sharing is clicked", function() {
      var hangupAllChatWindows = sandbox.stub(fakeMozLoop, "hangupAllChatWindows");

      var view = createTestComponent({
        inRoom: true,
        pendingOperation: false
      });

      var node = view.getDOMNode();
      TestUtils.Simulate.click(node.querySelector(".stop-sharing-button"));

      sinon.assert.calledOnce(hangupAllChatWindows);
    });
  });

  describe("loop.panel.SignInRequestView", function() {
    var view;

    function mountTestComponent() {
      return TestUtils.renderIntoDocument(
        React.createElement(loop.panel.SignInRequestView, {
          mozLoop: fakeMozLoop
        }));
    }

    it("should call login with forced re-authentication when sign-in is clicked", function() {
      view = mountTestComponent();

      TestUtils.Simulate.click(view.getDOMNode().querySelector("button"));

      sinon.assert.calledOnce(fakeMozLoop.logInToFxA);
      sinon.assert.calledWithExactly(fakeMozLoop.logInToFxA, true);
    });

    it("should logout when use as guest is clicked", function() {
      view = mountTestComponent();

      TestUtils.Simulate.click(view.getDOMNode().querySelector("a"));

      sinon.assert.calledOnce(fakeMozLoop.logOutFromFxA);
    });
  });

  describe("ConversationDropdown", function() {
    var view;

    function createTestComponent() {
      return TestUtils.renderIntoDocument(
        React.createElement(loop.panel.ConversationDropdown, {
          handleCopyButtonClick: sandbox.stub(),
          handleDeleteButtonClick: sandbox.stub(),
          handleEmailButtonClick: sandbox.stub(),
          eventPosY: 0
        }));
    }

    beforeEach(function() {
      view = createTestComponent();
    });

    it("should trigger handleCopyButtonClick when copy button is clicked",
       function() {
         TestUtils.Simulate.click(view.refs.copyButton.getDOMNode());

         sinon.assert.calledOnce(view.props.handleCopyButtonClick);
       });

    it("should trigger handleEmailButtonClick when email button is clicked",
       function() {
         TestUtils.Simulate.click(view.refs.emailButton.getDOMNode());

         sinon.assert.calledOnce(view.props.handleEmailButtonClick);
       });

    it("should trigger handleDeleteButtonClick when delete button is clicked",
       function() {
         TestUtils.Simulate.click(view.refs.deleteButton.getDOMNode());

         sinon.assert.calledOnce(view.props.handleDeleteButtonClick);
       });
  });

  describe("RoomEntryContextButtons", function() {
    var view, dispatcher;

    function createTestComponent(extraProps) {
      var props = _.extend({
        dispatcher: dispatcher,
        eventPosY: 0,
        showMenu: false,
        room: roomData,
        toggleDropdownMenu: sandbox.stub(),
        handleClick: sandbox.stub()
      }, extraProps);
      return TestUtils.renderIntoDocument(
        React.createElement(loop.panel.RoomEntryContextButtons, props));
    }

    beforeEach(function() {
      dispatcher = new loop.Dispatcher();
      sandbox.stub(dispatcher, "dispatch");

      view = createTestComponent();
    });

    it("should render ConversationDropdown if state.showMenu=true", function() {
      view = createTestComponent({ showMenu: true });

      expect(view.refs.menu).to.not.eql(undefined);
    });

    it("should not render ConversationDropdown by default", function() {
      view = createTestComponent({ showMenu: false });

      expect(view.refs.menu).to.eql(undefined);
    });

    it("should call toggleDropdownMenu after link is emailed", function() {
      view.handleEmailButtonClick(fakeEvent);

      sinon.assert.calledOnce(view.props.toggleDropdownMenu);
    });

    it("should call toggleDropdownMenu after conversation deleted", function() {
      view.handleDeleteButtonClick(fakeEvent);

      sinon.assert.calledOnce(view.props.toggleDropdownMenu);
    });

    it("should call toggleDropdownMenu after link is copied", function() {
      view.handleCopyButtonClick(fakeEvent);

      sinon.assert.calledOnce(view.props.toggleDropdownMenu);
    });

    it("should copy the URL when the callback is called", function() {
      view.handleCopyButtonClick(fakeEvent);

      sinon.assert.called(dispatcher.dispatch);
      sinon.assert.calledWithExactly(dispatcher.dispatch, new sharedActions.CopyRoomUrl({
        roomUrl: roomData.roomUrl,
        from: "panel"
      }));
    });

    it("should dispatch a delete action when callback is called", function() {
      view.handleDeleteButtonClick(fakeEvent);

      sinon.assert.calledWithExactly(dispatcher.dispatch,
        new sharedActions.DeleteRoom({ roomToken: roomData.roomToken }));
    });
  });
});
