#include "HaEntityClimate.h"
#include <HaUtilities.h>
#include <IJson.h>
#include <cstdlib>

#define COMPONENT "climate"
#define OBJECT_ID "climate"
#define OBJECT_ID_MODE "mode"
#define OBJECT_ID_TARGET "target"
#define OBJECT_ID_CURRENT "current"

HaEntityClimate::HaEntityClimate(HaBridge &ha_bridge, std::string name, std::string child_object_id,
                                 Configuration configuration)
    : _name(homeassistantentities::trim(name)), _ha_bridge(ha_bridge), _child_object_id(child_object_id),
      _configuration(configuration) {}

void HaEntityClimate::publishConfiguration() {
  IJsonDocument doc;

  if (!_name.empty()) {
    doc["name"] = _name;
  } else {
    doc["name"] = nullptr;
  }

  doc["retain"] = _configuration.retain;
  doc["min_temp"] = _configuration.min_temp;
  doc["max_temp"] = _configuration.max_temp;
  doc["temp_step"] = _configuration.temp_step;
  doc["temperature_unit"] = "C";

  JsonArrayType modes_array = createJsonArray(doc, "modes");
  for (const std::string &mode : _configuration.modes) {
    addToJsonArray(modes_array, mode);
  }

  doc["mode_state_topic"] =
      _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_MODE);
  doc["mode_command_topic"] =
      _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _child_object_id, OBJECT_ID_MODE);

  doc["temperature_state_topic"] =
      _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_TARGET);
  doc["temperature_command_topic"] =
      _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _child_object_id, OBJECT_ID_TARGET);

  doc["current_temperature_topic"] =
      _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_CURRENT);

  _ha_bridge.publishConfiguration(COMPONENT, OBJECT_ID, _child_object_id, doc);
}

void HaEntityClimate::republishState() {
  if (_mode) {
    publishMode(*_mode);
  }
  if (_target_temp) {
    publishTargetTemperature(*_target_temp);
  }
  if (_current_temp) {
    publishCurrentTemperature(*_current_temp);
  }
}

//--------------------------------------

void HaEntityClimate::publishCurrentTemperature(double temperature) {
  _current_temp = temperature;
  _ha_bridge.publishMessage(
      _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_CURRENT),
      std::to_string(temperature));
}

void HaEntityClimate::updateCurrentTemperature(double temperature) {
  if (!_current_temp || *_current_temp != temperature) {
    publishCurrentTemperature(temperature);
  }
}

//--------------------------------------

void HaEntityClimate::publishTargetTemperature(double temperature) {
  _target_temp = temperature;
  _ha_bridge.publishMessage(
      _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_TARGET),
      std::to_string(temperature));
}

void HaEntityClimate::updateTargetTemperature(double temperature) {
  if (!_target_temp || *_target_temp != temperature) {
    publishTargetTemperature(temperature);
  }
}

bool HaEntityClimate::setOnTargetTemperature(std::function<void(double)> callback) {
  return _ha_bridge.remote().subscribe(
      _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _child_object_id, OBJECT_ID_TARGET),
      [callback](std::string, std::string message) { callback(std::atof(message.c_str())); });
}

//--------------------------------------

void HaEntityClimate::publishMode(std::string mode) {
  _mode = mode;
  _ha_bridge.publishMessage(
      _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _child_object_id, OBJECT_ID_MODE), mode);
}

void HaEntityClimate::updateMode(std::string mode) {
  if (!_mode || *_mode != mode) {
    publishMode(mode);
  }
}

bool HaEntityClimate::setOnMode(std::function<void(std::string)> callback) {
  return _ha_bridge.remote().subscribe(
      _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _child_object_id, OBJECT_ID_MODE),
      [callback](std::string, std::string message) { callback(message); });
}
