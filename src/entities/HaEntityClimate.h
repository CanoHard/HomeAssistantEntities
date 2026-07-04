#ifndef __HA_ENTITY_CLIMATE_H__
#define __HA_ENTITY_CLIMATE_H__

#include <HaBridge.h>
#include <HaEntity.h>
#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <string>

/**
 * @brief Represent a Climate/Thermostat entity (Home Assistant "climate" component).
 *
 * Supports a current temperature (read only), a target temperature (setpoint) and
 * an HVAC mode (typically "off"/"heat"). This is enough to control a simple
 * heating thermostat from Home Assistant.
 */
class HaEntityClimate : public HaEntity {
public:
  struct Configuration {
    /**
     * @brief Supported HVAC modes. Reported and settable from Home Assistant.
     */
    std::set<std::string> modes = {"off", "heat"};

    /**
     * @brief Minimum settable target temperature.
     */
    double min_temp = 10.0;

    /**
     * @brief Maximum settable target temperature.
     */
    double max_temp = 40.0;

    /**
     * @brief Step used for the target temperature.
     */
    double temp_step = 0.5;

    /**
     * @brief If true, publish commands with retain set to true.
     */
    bool retain = false;
  };

  inline static Configuration _default = {
      .modes = {"off", "heat"}, .min_temp = 10.0, .max_temp = 40.0, .temp_step = 0.5, .retain = false};

  /**
   * @brief Construct a new Ha Entity Climate object.
   *
   * @param name human readable name shown in Home Assistant.
   * @param child_object_id child identifier (see other HaEntity types). Leave empty for none.
   * @param configuration the configuration for this entity.
   */
  HaEntityClimate(HaBridge &ha_bridge, std::string name, std::string child_object_id,
                  Configuration configuration = _default);

public:
  void publishConfiguration() override;
  void republishState() override;

  //--------------------------------------

  /**
   * @brief Publish the current (measured) temperature. Publishes regardless of change.
   */
  void publishCurrentTemperature(double temperature);

  /**
   * @brief Publish the current (measured) temperature, but only if it changed.
   */
  void updateCurrentTemperature(double temperature);

  //--------------------------------------

  /**
   * @brief Publish the target temperature (setpoint). Publishes regardless of change.
   */
  void publishTargetTemperature(double temperature);

  /**
   * @brief Publish the target temperature (setpoint), but only if it changed.
   */
  void updateTargetTemperature(double temperature);

  /**
   * @brief Set callback invoked when Home Assistant requests a new target temperature.
   */
  bool setOnTargetTemperature(std::function<void(double)> callback);

  //--------------------------------------

  /**
   * @brief Publish the HVAC mode (e.g. "off"/"heat"). Publishes regardless of change.
   */
  void publishMode(std::string mode);

  /**
   * @brief Publish the HVAC mode, but only if it changed.
   */
  void updateMode(std::string mode);

  /**
   * @brief Set callback invoked when Home Assistant requests a new HVAC mode.
   */
  bool setOnMode(std::function<void(std::string)> callback);

private:
  std::string _name;
  HaBridge &_ha_bridge;
  std::string _child_object_id;
  Configuration _configuration;

private:
  std::optional<double> _current_temp;
  std::optional<double> _target_temp;
  std::optional<std::string> _mode;
};

#endif // __HA_ENTITY_CLIMATE_H__
