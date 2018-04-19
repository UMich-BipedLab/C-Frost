/*
 * Automatically Generated from Mathematica.
 * <* DateString[] <> " " <> DateString["TimeZoneName"]  *>
 */

#ifndef <*ToUpperCase[TemplateSlot["name"]]*>_HH
#define <*ToUpperCase[TemplateSlot["name"]]*>_HH

namespace frost {
    namespace gen {
        void <*TemplateSlot["name"]*>(<*StringImplode[Table["double *p_" <> TemplateSlot["argouts"][[i]], {i, Length[TemplateSlot["argouts"]]}], ", "]*>, <*StringImplode[Table["const double *"<>ToString[arg], {arg, TemplateSlot["argins"]}], ","]*>);
    }
}

#endif // <*ToUpperCase[TemplateSlot["name"]]*>_HH
