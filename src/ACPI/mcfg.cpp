#include <AlopexOS/ACPI/mcfg.hpp>

auto AlopexOS::MCFG::is_mcfg_signature(const char *sig) -> bool {
    return sig[0] == 'M' && sig[1] == 'C' && sig[2] == 'F' && sig[3] == 'G';
}