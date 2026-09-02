Scriptname DynamicArmorPhysics Hidden

Int[] Function GetVersion() global native

Bool Function GetIniValueBool(String asValue, Bool abDefaultValue = false) global native

Float Function GetIniValueFloat(String asValue, Float afDefaultValue = 0.0) global native

Int Function GetIniValueInt(String asValue, Int aiDefaultValue = 0) global native

String Function GetIniValueString(String asValue, String asDefaultValue = "") global native

Bool Function GetDefaultIniValueBool(String asValue, Bool abFallback = false) global native

Float Function GetDefaultIniValueFloat(String asValue, Float afFallback = 0.0) global native

Int Function GetDefaultIniValueInt(String asValue, Int aiFallback = 0) global native

String Function GetDefaultIniValueString(String asValue, String asFallback = "") global native

Bool Function SetIniValueBool(String asKeySection, Bool abValue) global native

Bool Function SetIniValueFloat(String asKeySection, Float afValue) global native

Bool Function SetIniValueInt(String asKeySection, Int aiValue) global native

Bool Function SetIniValueString(String asKeySection, String asValue) global native

Function RequestRuntimeUpdate() global native