

#ifndef PluginDataChromium_h
#define PluginDataChromium_h

#include "PluginData.h"

namespace WebCore {

// Checks if any of the plugins handle this extension, and if so returns the
// plugin's mime type for this extension.  Otherwise returns an empty string.
String getPluginMimeTypeFromExtension(const String& extension);

} // namespace WebCore

#endif // PluginDataChromium_h
