#include "hyrisc/hyrisc.hpp"

#include <csignal>
#include <iomanip>
#include <string>

#include "log.hpp"

#include "dev/flash.hpp"
#include "dev/terminal.hpp"
#include "dev/memory.hpp"
#include "dev/bios.hpp"
#include "dev/iobus.hpp"
#include "dev/iobus/pci.hpp"
#include "dev/iobus/ata.hpp"

hyrisc_t* cpu = new hyrisc_t;

const char* bus_error_codes[] = {
    "HY_EOK", // EPERM
    "HY_ENOENT",
    "HY_ESRCH",
    "HY_EINTR",
    "HY_EIO",
    "HY_ENXIO",
    "HY_E2BIG",
    "HY_ENOEXEC",
    "HY_EBADF",
    "HY_ECHILD",
    "HY_EAGAIN",
    "HY_ENOMEM",
    "HY_EACCES",
    "HY_EFAULT",
    "HY_ENOTBLK",
    "HY_EBUSY",
    "HY_EEXIST",
    "HY_EXDEV",
    "HY_ENODEV",
    "HY_ENOTDIR",
    "HY_EISDIR",
    "HY_EINVAL",
    "HY_ENFILE",
    "HY_EMFILE",
    "HY_ENOTTY",
    "HY_ETXTBSY",
    "HY_EFBIG",
    "HY_ENOSPC",
    "HY_ESPIPE",
    "HY_EROFS",
    "HY_EMLINK",
    "HY_EPIPE",
    "HY_EDOM",
    "HY_ERANGE",
    "HY_EDEADLK",
    "HY_ENAMETOOLONG",
    "HY_ENOLCK",
    "HY_ENOSYS",
    "HY_ENOTEMPTY",
    "HY_ELOOP",
    "HY_ENOMSG",
    "HY_EIDRM",
    "HY_ECHRNG",
    "HY_EL2NSYNC",
    "HY_EL3HLT",
    "HY_EL3RST",
    "HY_ELNRNG",
    "HY_EUNATCH",
    "HY_ENOCSI",
    "HY_EL2HLT",
    "HY_EBADE",
    "HY_EBADR",
    "HY_EXFULL",
    "HY_ENOANO",
    "HY_EBADRQC",
    "HY_EBADSLT",
    "HY_EBFONT",
    "HY_ENOSTR",
    "HY_ENODATA",
    "HY_ETIME",
    "HY_ENOSR",
    "HY_ENONET",
    "HY_ENOPKG",
    "HY_EREMOTE",
    "HY_ENOLINK",
    "HY_EADV",
    "HY_ESRMNT",
    "HY_ECOMM",
    "HY_EPROTO",
    "HY_EMULTIHOP",
    "HY_EDOTDOT",
    "HY_EBADMSG",
    "HY_EOVERFLOW",
    "HY_ENOTUNIQ",
    "HY_EBADFD",
    "HY_EREMCHG",
    "HY_ELIBACC",
    "HY_ELIBBAD",
    "HY_ELIBSCN",
    "HY_ELIBMAX",
    "HY_ELIBEXEC",
    "HY_EILSEQ",
    "HY_ERESTART",
    "HY_ESTRPIPE",
    "HY_EUSERS",
    "HY_ENOTSOCK",
    "HY_EDESTADDRREQ",
    "HY_EMSGSIZE",
    "HY_EPROTOTYPE",
    "HY_ENOPROTOOPT",
    "HY_EPROTONOSUPPORT",
    "HY_ESOCKTNOSUPPORT",
    "HY_EOPNOTSUPP",
    "HY_EPFNOSUPPORT",
    "HY_EAFNOSUPPORT",
    "HY_EADDRINUSE",
    "HY_EADDRNOTAVAIL",
    "HY_ENETDOWN",
    "HY_ENETUNREACH",
    "HY_ENETRESET",
    "HY_ECONNABORTED",
    "HY_ECONNRESET",
    "HY_ENOBUFS",
    "HY_EISCONN",
    "HY_ENOTCONN",
    "HY_ESHUTDOWN",
    "HY_ETOOMANYREFS",
    "HY_ETIMEDOUT",
    "HY_ECONNREFUSED",
    "HY_EHOSTDOWN",
    "HY_EHOSTUNREACH",
    "HY_EALREADY",
    "HY_EINPROGRESS",
    "HY_ESTALE",
    "HY_EUCLEAN",
    "HY_ENOTNAM",
    "HY_ENAVAIL",
    "HY_EISNAM",
    "HY_EREMOTEIO",
    "HY_EDQUOT",
    "HY_ENOMEDIUM",
    "HY_EMEDIUMTYPE",
    "HY_ECANCELED",
    "HY_ENOKEY",
    "HY_EKEYEXPIRED",
    "HY_EKEYREVOKED",
    "HY_EKEYREJECTED",
    "HY_EOWNERDEAD",
    "HY_ENOTRECOVERABLE"
};

void print_cpu_status_main() {
    _log(info, "State:\nMain registers:");

    for (int r = 0; r < 32; r++) {
        if (!(r % 4) && r) {
            std::cout << std::endl;
        }

        std::string name = hyrisc_register_names[r];

        std::cout << name << std::string(4 - name.size(), ' ') << ": "
                  << "0x" << std::setw(8) << std::setfill('0') << std::hex << cpu->internal.r[r] << "  ";
    }

    std::cout << "\nFlags      : ----"
              << ((cpu->internal.st & 0b00001000) ? 'C' : 'c')
              << ((cpu->internal.st & 0b00000100) ? 'V' : 'v')
              << ((cpu->internal.st & 0b00000010) ? 'N' : 'n')
              << ((cpu->internal.st & 0b00000001) ? 'Z' : 'z')
              << std::endl;
    std::cout << "Cycle      : " << std::dec << (int)cpu->internal.cycle << std::endl;
    std::cout << "Instruction: " << std::setw(8) << std::setfill('0') << std::hex << cpu->internal.instruction << std::endl;
    //std::cout << "Link level : " << std::dec << cpu->internal.link_level << std::endl;
}

void print_cpu_status() {
    _log(info, "State:\nMain registers:");

    for (int r = 0; r < 32; r++) {
        if (!(r % 4) && r) {
            std::cout << std::endl;
        }

        std::string name = hyrisc_register_names[r];

        std::cout << name << std::string(4 - name.size(), ' ') << ": "
                  << "0x" << std::setw(8) << std::setfill('0') << std::hex << cpu->internal.r[r] << "  ";
    }

    std::cout << "\nFlags      : ----"
              << ((cpu->internal.st & 0b00001000) ? 'C' : 'c')
              << ((cpu->internal.st & 0b00000100) ? 'V' : 'v')
              << ((cpu->internal.st & 0b00000010) ? 'N' : 'n')
              << ((cpu->internal.st & 0b00000001) ? 'Z' : 'z')
              << std::endl;
    std::cout << "Cycle      : " << std::dec << cpu->internal.cycle << std::endl;
    std::cout << "Instruction: " << std::setw(8) << std::setfill('0') << std::hex << cpu->internal.instruction << std::endl;
    //std::cout << "Link level : " << std::dec << cpu->internal.link_level << std::endl;

    std::cout << "\nFloating Point registers:\n";

    std::cout.precision(7);
    
    for (int f = 0; f < 32; f++) {
        if (!(f % 4) && f) {
            std::cout << std::endl;
        }

        std::string name = "f" + std::to_string(f);

        std::cout << name << std::string(3 - name.size(), ' ') << ": "
                  << std::showpos << std::fixed << cpu->internal.f[f] << "  ";
    }

    std::cout << "\n\nPins:\n";

    std::cout << std::noshowpos;

    std::cout << "Main bus (BCI): (status: ";
    
    if (cpu->ext.bci.rw) {
        hyu32_t value = cpu->ext.bci.d & ((cpu->ext.bci.s == 0) ? 0xff : ((cpu->ext.bci.s == 1) ? 0xffff : 0xffffffff));

        std::cout << "Write 0x"
                  << std::setw(((cpu->ext.bci.s == 0) ? 2 : ((cpu->ext.bci.s == 1) ? 4 : 8)))
                  << std::setfill('0') << std::hex << value
                  << " to address "
                  << std::setw(8) << std::setfill('0') << std::hex << cpu->ext.bci.a
                  << " as "
                  << ((cpu->ext.bci.s == 0) ? "byte" : ((cpu->ext.bci.s == 1) ? "16-bit word" : "32-bit word"));
    } else {
        std::cout << "Read "
                  << ((cpu->ext.bci.s == 0) ? "byte" : ((cpu->ext.bci.s == 1) ? "16-bit word" : "32-bit word"))
                  << " at address 0x" << std::setw(8) << std::setfill('0') << std::hex << cpu->ext.bci.a;
    }

    std::cout << ")\n";

    std::cout << "A0-A31 : 0x" << std::setw(8) << std::setfill('0') << std::hex << cpu->ext.bci.a << std::endl;
    std::cout << "D0-D31 : 0x" << std::setw(8) << std::setfill('0') << std::hex << cpu->ext.bci.d << std::endl;
    std::cout << "S0-S1  : " << std::dec << (int)cpu->ext.bci.s << std::endl;
    std::cout << "BE0-BE7: 0x" << std::setw(2) << std::setfill('0') << std::hex << cpu->ext.bci.be;
    std::cout << " (" << bus_error_codes[cpu->ext.bci.be] << ")\n";
    std::cout << "RW     : " << (cpu->ext.bci.rw ? "high" : "low") << std::endl;
    std::cout << "BUSREQ : " << (cpu->ext.bci.busreq ? "high" : "low") << std::endl;
    std::cout << "BUSACK : " << (cpu->ext.bci.busack ? "high" : "low") << std::endl;
    std::cout << "BUSIRQ : " << (cpu->ext.bci.busirq ? "high" : "low") << std::endl;

    std::cout << "\nInterrupts (PIC):\n";
    std::cout << "V0-V31: 0x" << std::setw(8) << std::setfill('0') << std::hex << cpu->ext.pic.v << std::endl;
    std::cout << "IRQ   : " << (cpu->ext.pic.irq ? "high" : "low") << std::endl;
    std::cout << "IRQACK: " << (cpu->ext.pic.irqack ? "high" : "low") << std::endl;

    std::cout << "\nCPU control:\n";
    std::cout << "FREEZE: " << (cpu->ext.freeze ? "high" : "low") << std::endl;
    std::cout << "RESET : " << (cpu->ext.reset ? "high" : "low") << std::endl;
    std::cout << "VCC   : " << cpu->ext.vcc << std::endl;
}

void sigill_handler(int signal) {
    if (cpu->id) {
        _log(info, "%s executed an illegal instruction!", cpu->id);
    } else {
        _log(info, "CPU%u executed an illegal instruction!", cpu->core);
    }

    print_cpu_status();

    std::exit(1);
}

void sigint_handler(int signal) {
    //std::cout << std::endl;

    if (cpu->id) {
        _log(info, "%s killed!", cpu->id);
    } else {
        _log(info, "CPU%u killed!", cpu->core);
    }

    print_cpu_status_main();

    std::exit(0);
}

void sigbreak_handler(int signal) {
    //std::cout << std::endl;

    if (cpu->id) {
        _log(info, "Break requested by %s!", cpu->id);
    } else {
        _log(info, "Break requested by CPU%u!", cpu->core);
    }

    print_cpu_status();

    std::exit(0);
}

void sigfpe_handler(int signal) {
    if (cpu->id) {
        _log(info, "%s triggered a floating point exception!", cpu->id);
    } else {
        _log(info, "CPU%u triggered a floating point exception!", cpu->core);
    }

    print_cpu_status();

    std::exit(0);
}

std::vector <device_t*> hardware;

void add_hardware(device_t* dev) {
    hardware.push_back(dev);
};

int main(int argc, const char* argv[]) {
    std::signal(SIGFPE, sigfpe_handler);
    std::signal(SIGINT, sigint_handler);
    std::signal(SIGILL, sigill_handler);
    std::signal(SIGBREAK, sigbreak_handler);

    _log::init("hyrisc");

    dev_terminal_t terminal;
    //dev_memory_t memory;
    dev_bios_t bios;

    bios.create(0x1000, 0x80000000);
    bios.init(&cpu->ext);
    bios.load("pci.elf", true);

    // flash.create(0x10000, 0x90000000);
    // flash.init(&cpu->ext);
    // flash.load("program.bin");

    terminal.create(0xa0000000);
    terminal.init(&cpu->ext);

    dev_iobus_t iobus;
    iobus_dev_pci_t pci;
    iobus_dev_ata_t ide;


    /*           a0000000    fffffffe
    System bus -----+------------+-
                    |            |        1f0   cf8
                    terminal     iobus ----+-----+--------
                                           |     |
                                           ide   pci -+-----------
                                           |          |
                                           +--------> bus 0, device 0
    */

    iobus.init(&cpu->ext);
    iobus.attach_device(&pci);
    iobus.attach_device(&ide);
    pci.register_device(ide.get_pci_desc(), 0, 0);

    if (!ide.attach_drive("test.img", ATA_PRI_MASTER)) {
        _log(error, "Couldn't attach drive with image \"%s\" to ATA channel", "test.img");
    }

    add_hardware(&terminal);
    add_hardware(&iobus);
    add_hardware(&bios);

    // memory.create(0x10000, 0x10000000);
    // memory.init(&cpu->ext);

    hyrisc_set_cpuid(cpu, "main-cpu", 0);
    hyrisc_pulse_reset(cpu, 0x80000000);

    cpu->ext.bci.busirq = false;
    cpu->ext.vcc = 1.0f;

    while (true) {
        hyrisc_clock(cpu);
        
        for (device_t* dev : hardware)
            dev->update();
    }

    print_cpu_status_main();
}