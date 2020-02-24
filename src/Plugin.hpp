////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_FLY_TO_LOCATIONS_PLUGIN_HPP
#define CSP_FLY_TO_LOCATIONS_PLUGIN_HPP

#include "../../../src/cs-core/PluginBase.hpp"

#include <map>
#include <string>
#include <vector>
#include <optional>

namespace csp::flytolocations {

/// This plugin enables the user to travel to different locations within the solar system. The
/// locations are organized in a flat hierarchy. There are targets, which can be any anchor in
/// the solar system and there are locations, which are places belonging to a target. The
/// locations can be organized into groups.
///
/// As an example the Earth can be a target. New York, Berlin and Hannover can be locations in
/// the "Cities" group belonging to the target Earth.
///
/// The configuration of this plugin is done via the provided json config.
/// See README.md for details.
class Plugin : public cs::core::PluginBase {
 public:
  struct Settings {

    /// Settings for a location within a target.
    struct Location {
      double      mLatitude;  ///< The latitude of the location.
      double      mLongitude; ///< The longitude of the location.
      double      mExtent;    ///< The location size in meters.
      std::string mGroup;     ///< The name of the group the location belongs to.
    };

    struct Minimap {
      std::string                mMapServer;
      std::optional<std::string> mLayer;
      std::optional<std::string> mAttribution;
      double                     mCircumference;
    };

    /// Settings for a single target.
    struct Target {
      std::string                     mIcon;      ///< The path to an icon for the target.
      std::optional<Minimap>          mMinimap;
      std::map<std::string, Location> mLocations; ///< All the locations belonging to the target.
    };

    std::map<std::string, Target> mTargets;
  };

  Plugin();

  void init() override;
  void deInit() override;

 private:
  Settings mPluginSettings;

  int mActiveBodyConnection = -1;
};

} // namespace csp::flytolocations

#endif // CSP_FLY_TO_LOCATIONS_PLUGIN_HPP
