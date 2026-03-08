#ifndef __HA_ENTITY_VALVE_H__
#define __HA_ENTITY_VALVE_H__

#include <HaBridge.h>
#include <HaEntity.h>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

/**
 * @brief Represent an MQTT Valve that can be controlled from Home Assistant.
 * Supports state-based and position-based modes.
 * See https://www.home-assistant.io/integrations/valve.mqtt/
 */
class HaEntityValve : public HaEntity {
public:
  struct Configuration {
    /**
     * @brief If true, the valve reports and accepts a numeric position (0-100).
     * Commands will be numeric positions instead of OPEN/CLOSE/STOP payloads.
     */
    bool reports_position = false;

    /**
     * @brief Number representing the closed position (default 0).
     */
    uint8_t position_closed = 0;

    /**
     * @brief Number representing the open position (default 100).
     */
    uint8_t position_open = 100;

    /**
     * @brief If true, this tells Home Assistant to publish the message on the
     * command topic with retain set to true.
     */
    bool retain = false;

    /**
     * @brief Optional per-entity availability topic. When set, overrides the
     * global availability topic from HaBridge. Leave empty to use the global one.
     */
    std::string availability_topic = "";

    std::string device_class = ""; 
  };

  inline static Configuration _default = {
      .reports_position = false, .position_closed = 0, .position_open = 100, .retain = false, .availability_topic = "", .device_class = ""};

  /**
   * @brief Construct a new HaEntityValve object.
   *
   * @param ha_bridge the HaBridge to use for publishing.
   * @param name human readable name for the entity in Home Assistant. Can be
   * empty if the device name alone is sufficient.
   * @param child_object_id optional child identifier when multiple valves share
   * the same node ID. Valid characters: [a-zA-Z0-9_-]. Leave empty for none.
   * @param configuration the configuration for this entity.
   */
  HaEntityValve(HaBridge &ha_bridge, std::string name, std::string child_object_id,
                Configuration configuration = _default);

public:
  void publishConfiguration() override;
  void republishState() override;

  enum class State {
    Open,
    Opening,
    Closed,
    Closing,
  };

  /**
   * @brief Publish the valve state and/or position. Always publishes to MQTT.
   * Also see update().
   *
   * @param state the valve state, or std::nullopt if unknown.
   * @param position the valve position (0-100), only used when reports_position
   * is true. Ignored when reports_position is false.
   */
  void publish(std::optional<State> state, std::optional<uint8_t> position = std::nullopt, bool retain = false);

  /**
   * @brief Publish the valve state and/or position, but only if the value has
   * changed. Also see publish().
   */
  void update(std::optional<State> state, std::optional<uint8_t> position = std::nullopt, bool retain = false);

  enum class Action {
    Open,
    Close,
    Stop,
    Unknown,
  };

  /**
   * @brief Set callback for receiving commands when reports_position is false.
   * Callback receives Open, Close or Stop actions.
   */
  bool setOnCommand(std::function<void(Action)> callback);

  /**
   * @brief Set callback for receiving the target position when reports_position
   * is true. Value is between position_closed and position_open.
   */
  bool setOnPosition(std::function<void(uint8_t)> callback);

private:
  std::string _name;
  HaBridge &_ha_bridge;
  std::string _child_object_id;
  Configuration _configuration;

private:
  std::optional<State> _state;
  std::optional<uint8_t> _position;
};

#endif // __HA_ENTITY_VALVE_H__
