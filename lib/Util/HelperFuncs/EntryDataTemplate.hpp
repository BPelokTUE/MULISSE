#ifndef UTIL_HELPERFUNCS_ENTRYDATATEMPLATE_HPP
#define UTIL_HELPERFUNCS_ENTRYDATATEMPLATE_HPP

#define DECLARE_ENTRY_DATA_SPECS(CLASS) \
    template class CLASS<Paa>;          \
    template class CLASS<Envelope>;

#define DECLARE_FINALIZED_TAG_SPECS(CLASS) \
    template class CLASS<PaaTag>;          \
    template class CLASS<EnvelopeTag>;

#endif  // UTIL_HELPERFUNCS_ENTRYDATATEMPLATE_HPP
