#include <fennlib/sys>
#include <skeleton.hpp>
#include <fennlib/types>
#include <boot/bootPixel.hpp>
#include <boot/bootConsole.hpp>
#include <displayManager.hpp>
#include <synapse.hpp>
#include <logger.hpp>
#include <kernel/DRAMA.hpp>
#include <SystemX/systemx.hpp>
#include <AlopexOS/avfs.hpp>
#include <AlopexOS/btrfs.hpp>

// testing library
#include <SystemX/test_programs.hpp>

using namespace fennlib;

namespace {
    auto number_to_string(u32 val) -> fennlib::string {
        if (val == 0) {
            return "0";
        }
        char buffer[32];
        int idx = 31;
        buffer[idx] = '\0';
        while (val > 0 && idx > 0) {
            buffer[--idx] = '0' + (val % 10);
            val /= 10;
        }
        return &buffer[idx];
    }
}

extern "C" void kmain(void) {
    io::Skeleton skeleton;
    skeleton.init();

    io::Logger logger;
    logger.init();

    graphics::DisplayManager displayManager;

    displayManager.update(skeleton);

    if (!displayManager.getPrimaryDisplay()) skeleton.halt();

    u32 frame_counter = 0;

    io::Synapse synapse;
    bool synapse_initialized = false;

    AlopexOS::Kernel::DRAMA drama(&synapse);
    SystemX system_x(&drama);
    bool drama_initialized = false;
    bool systemx_initialized = false;
    bool avfs_initialized = false;
    bool btrfs_tested = false;

    if constexpr (sys::bootloader::is_limine) {
        bootConsole::write_boot_log("bootloader: Limine");
    }

    for (;;) {
        if (!synapse_initialized) {
            bootConsole::write_boot_log("Initializing: synapse!...");
            synapse.init(skeleton, logger);
            synapse_initialized = true;
            bootConsole::write_boot_log("Initialized: synapse.");
            
            bootConsole::write_boot_log("--- Logger Diagnostic Dump ---");
            for (const auto& entry : logger.get_entries()) {
                bootConsole::write_boot_log(entry.message);
            }
            bootConsole::write_boot_log("------------------------------");
            
            fennlib::string mfr_msg = "Board Mfr: ";
            mfr_msg += synapse.get_root_motherboard().get_manufacturer().c_str();
            bootConsole::write_boot_log(mfr_msg.c_str());

            fennlib::string name_msg = "Board Name: ";
            name_msg += synapse.get_root_motherboard().get_name().c_str();
            bootConsole::write_boot_log(name_msg.c_str());

            fennlib::string cpu_count_msg = "CPU Count: ";
            cpu_count_msg += number_to_string(synapse.get_cpus().size()).c_str();
            bootConsole::write_boot_log(cpu_count_msg.c_str());

            for (const auto& desc : synapse.get_registry()) {
                if (desc.type == io::DeviceClass::CPU && desc.active) {
                    fennlib::string cpu_rtt = "CPU UID 0x";
                    cpu_rtt += number_to_string(desc.uid).c_str();
                    cpu_rtt += " RTT (CCT): ";
                    cpu_rtt += number_to_string(static_cast<u32>(desc.rtt)).c_str();
                    bootConsole::write_boot_log(cpu_rtt.c_str());
                }
            }

            int idx = 0;
            for (const auto& cpu : synapse.get_cpus()) {
                fennlib::string cpu_mfr = "CPU ";
                cpu_mfr += number_to_string(idx).c_str();
                cpu_mfr += " Mfr: ";
                cpu_mfr += cpu.get_manufacturer().c_str();
                bootConsole::write_boot_log(cpu_mfr.c_str());

                fennlib::string cpu_ver = "CPU ";
                cpu_ver += number_to_string(idx).c_str();
                cpu_ver += " Ver: ";
                cpu_ver += cpu.get_version().c_str();
                bootConsole::write_boot_log(cpu_ver.c_str());

                fennlib::string cpu_cores = "CPU ";
                cpu_cores += number_to_string(idx).c_str();
                cpu_cores += " Cores: ";
                cpu_cores += number_to_string(cpu.get_core_count()).c_str();
                bootConsole::write_boot_log(cpu_cores.c_str());

                fennlib::string cpu_threads = "CPU ";
                cpu_threads += number_to_string(idx).c_str();
                cpu_threads += " Threads: ";
                cpu_threads += number_to_string(cpu.get_thread_count()).c_str();
                bootConsole::write_boot_log(cpu_threads.c_str());

                idx++;
            }

            fennlib::string ram_count_msg = "RAM Module Count: ";
            ram_count_msg += number_to_string(synapse.get_ram_modules().size()).c_str();
            bootConsole::write_boot_log(ram_count_msg.c_str());

            int ram_idx = 0;
            for (const auto& ram : synapse.get_ram_modules()) {
                if (!ram.is_populated()) {
                    ram_idx++;
                    continue;
                }

                fennlib::string ram_loc = "RAM ";
                ram_loc += number_to_string(ram_idx).c_str();
                ram_loc += " Locator: ";
                ram_loc += ram.get_device_locator().c_str();
                bootConsole::write_boot_log(ram_loc.c_str());

                fennlib::string ram_size = "RAM ";
                ram_size += number_to_string(ram_idx).c_str();
                ram_size += " Size (MB): ";
                ram_size += number_to_string(static_cast<u32>(ram.get_size_mb())).c_str();
                bootConsole::write_boot_log(ram_size.c_str());

                fennlib::string ram_spd = "RAM ";
                ram_spd += number_to_string(ram_idx).c_str();
                ram_spd += " Speed (MHz): ";
                ram_spd += number_to_string(ram.get_speed_mhz()).c_str();
                bootConsole::write_boot_log(ram_spd.c_str());

                ram_idx++;
            }
        }

        if (!drama_initialized) {
            bootConsole::write_boot_log("Initializing: DRAMA memory manager!...");
            drama.init<true>(&logger);

            u8* ptr = reinterpret_cast<u8*>(drama.malloc(8, 0));

            ptr[1] = 2;

            if (ptr[1] == 2) {
                bootConsole::write_boot_log("Malloc is operational!");
            }

            drama_initialized = true;
            bootConsole::write_boot_log("Initialized: DRAMA.");
        }

        if (!avfs_initialized) {
            bootConsole::write_boot_log("Initializing: AVFS!...");
            avfs::init(&synapse);
            avfs_initialized = true;
            bootConsole::write_boot_log("Initialized: AVFS.");
        }

        if (avfs_initialized && !btrfs_tested) {
            bootConsole::write_boot_log("Testing Btrfs reformat & mount...");

            AlopexOS::types::UID target_uid = fennlib::types::nil;
            for (const auto& desc : synapse.get_registry()) {
                if (desc.active && desc.mmio_base != 0) {
                    target_uid = desc.uid;
                    break;
                }
            }

            if (target_uid != fennlib::types::nil) {
                bootConsole::write_boot_log("Found active device target for Btrfs test.");

                btrfs::FileSystem test_fs;
                auto format_err = test_fs.reformat<true>(&synapse, target_uid, &logger);
                if (format_err == AlopexOS::error_code::Success) {
                    bootConsole::write_boot_log("Btrfs test: Reformat succeeded.");
                } else {
                    bootConsole::write_boot_log("Btrfs test: Reformat failed.");
                }

                auto mount_err = test_fs.mount<true>(&synapse, target_uid, &logger);
                if (mount_err == AlopexOS::error_code::Success && test_fs.is_mounted()) {
                    bootConsole::write_boot_log("Btrfs test: Direct mount succeeded!");
                } else {
                    bootConsole::write_boot_log("Btrfs test: Direct mount failed.");
                }

                // 2. AVFS Interface Mount Test
                auto avfs_mount_err = avfs::g_interface->mount<true>(target_uid, &logger);
                if (avfs_mount_err == AlopexOS::error_code::Success) {
                    bootConsole::write_boot_log("AVFS test: Interface mount succeeded!");
                } else {
                    bootConsole::write_boot_log("AVFS test: Interface mount failed.");
                }
            } else {
                bootConsole::write_boot_log("Btrfs test skipped: No active MMIO device found in registry.");
            }

            btrfs_tested = true;
        }

        if (!systemx_initialized) {
            bootConsole::write_boot_log("Initializing: SystemX!...");
            
            usize exit_code = system_x.invoke(
                test_programs::program_start(test_programs::sample_program),
                test_programs::program_end(test_programs::sample_program)
            );

            string exit_code_text = number_to_string(exit_code);
            bootConsole::write_boot_log(string(string("Return Value of test program: ") + exit_code_text).c_str());

            bootConsole::write_boot_log("Initialized: SystemX");

            systemx_initialized = true;
        }

        auto* primary = displayManager.getPrimaryDisplay();
        if (primary) {
            primary->clear(0x00000000);
            
            bootConsole::flush(primary->getBackBuffer(), primary->getPitch(), 10, 10, 0xFFFFFFFF);
            
            u32 indicator_step = frame_counter % 64;

            bootPixel::drawLoadingIndicator(
                primary->getBackBuffer(),
                primary->getPitch(),
                (primary->getWidth() - 256) / 2,
                (primary->getHeight() - 256) / 2,
                indicator_step
            );
            
            displayManager.flushAll();
        }

        frame_counter++;
    }
    
    skeleton.halt();
}