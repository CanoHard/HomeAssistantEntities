#include "HaEntityValve.h"
#include <HaUtilities.h>
#include <IJson.h>
#include <algorithm>
#include <cstdlib>

// NOTE! We have swapped object ID and child object ID to get a nicer state/command topic path.
#define COMPONENT "valve"
#define OBJECT_ID "valve"
#define OBJECT_ID_STATE "state"

using namespace homeassistantentities;

HaEntityValve::HaEntityValve(HaBridge &ha_bridge, std::string name, std::string child_object_id,
                             Configuration configuration)
    : _name(homeassistantentities::trim(name)), _ha_bridge(ha_bridge), _child_object_id(child_object_id),
      _configuration(configuration) {}

void HaEntityValve::publishConfiguration() {
  IJsonDocument doc;

  if (!_name.empty()) {
    doc["name"] = _name;
  } else {
    doc["name"] = nullptr;
  }

  doc["retain"] = _configuration.retain;
  doc["reports_position"] = _configuration.reports_position;

  if (_configuration.reports_position) {
    doc["position_open"] = _configuration.position_open;
    doc["position_closed"] = _configuration.position_closed;
  }

  doc["state_topic"] = _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_STATE);
  doc["command_topic"] =
      _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _child_object_id, OBJECT_ID_STATE);

  if (!_configuration.availability_topic.empty()) {
    doc["availability_topic"] = _configuration.availability_topic;
  }

  _ha_bridge.publishConfiguration(COMPONENT, OBJECT_ID, _child_object_id, doc);
}

void HaEntityValve::republishState() { publish(_state, _position); }

static std::string stateToString(HaEntityValve::State state) {
  switch (state) {
  case HaEntityValve::State::Open:
    return "open";
  case HaEntityValve::State::Opening:
    return "opening";
  case HaEntityValve::State::Closed:
    return "closed";
  case HaEntityValve::State::Closing:
    return "closing";
  }
  return "";
}

void HaEntityValve::publish(std::optional<State> state, std::optional<uint8_t> position) {
  if (!state && !position) {
    return;
  }

  std::string topic = _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_STATE);

  if (_configuration.reports_position) {
    // Publish as JSON: {"state": "opening", "position": 10}
    IJsonDocument doc;
    if (state) {
      std::string str = stateToString(*state);
      if (!str.empty()) {
        doc["state"] = str;
        _state = state;
      }
    }
    if (position) {
      uint8_t lo = _configuration.position_closed;
      uint8_t hi = _configuration.position_open;
      if (lo > hi) {
        std::swap(lo, hi);
      }
      uint8_t clamped = std::min(std::max(*position, lo), hi);
      doc["position"] = clamped;
      _position = clamped;
    }
    if (!doc.empty()) {
      _ha_bridge.publishMessage(topic, toJsonString(doc));
    }
  } else {
    // Publish as plain state string
    if (state) {
      std::string str = stateToString(*state);
      if (!str.empty()) {
        _ha_bridge.publishMessage(topic, str);
        _state = state;
      }
    }
  }
}

void HaEntityValve::update(std::optional<State> state, std::optional<uint8_t> position) {
  bool state_changed = state != _state;
  bool position_changed = position != _position;
  if (state_changed || position_changed) {
    publish(state_changed ? state : std::nullopt, position_changed ? position : std::nullopt);
  }
}

bool HaEntityValve::setOnCommand(std::function<void(Action)> callback) {
  return _ha_bridge.remote().subscribe(
      _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _child_object_id, OBJECT_ID_STATE),
      [callback](std::string topic, std::string message) {
        Action action = Action::Unknown;
        if (message == "OPEN") {
          action = Action::Open;
        } else if (message == "CLOSE") {
          action = Action::Close;
        } else if (message == "STOP") {
          action = Action::Stop;
        }
        callback(action);
      });
}

bool HaEntityValve::setOnPosition(std::function<void(uint8_t)> callback) {
  return _ha_bridge.remote().subscribe(
      _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _child_object_id, OBJECT_ID_STATE),
      [callback](std::string topic, std::string message) { callback(static_cast<uint8_t>(std::atoi(message.c_str()))); });
}
