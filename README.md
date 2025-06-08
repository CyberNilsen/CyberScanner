# CyberScanner

A lightweight TCP port scanner built with Qt and C++ for network reconnaissance and security testing.

## CyberScanner Preview

![CyberScanner Preview](https://github.com/user-attachments/assets/37e3f3f1-4877-4f72-947a-a9e3b837c322)


## Features

- **TCP Port Scanning**: Efficient scanning of TCP ports on target hosts
- **Graphical User Interface**: User-friendly Qt-based interface with custom icons
- **Cross-Platform**: Built with CMake for compatibility across different operating systems
- **Fast Performance**: Optimized scanning algorithms for quick results

## Requirements

- **Qt Framework**: Qt5 or Qt6
- **CMake**: Version 3.5 or higher
- **C++ Compiler**: Supporting C++11 or later (GCC, Clang, or MSVC)

## Installation

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/CyberNilsen/CyberScanner.git
   cd CyberScanner
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure with CMake:
   ```bash
   cmake ..
   ```

4. Build the project:
   ```bash
   make
   ```

5. Run the application:
   ```bash
   ./CyberScanner
   ```

## Usage

1. Launch the CyberScanner application
2. Enter the target IP address or hostname
3. Specify the port range to scan
4. Click "Start Scan" to begin the TCP port scan
5. View results in the interface showing open/closed ports

## Project Structure

```
CyberScanner/
├── .github/workflows/    # CI/CD configuration
├── Images/              # Application icons and images
├── main.cpp            # Application entry point
├── mainwindow.cpp      # Main window implementation
├── mainwindow.h        # Main window header
├── mainwindow.ui       # Qt UI design file
├── resources.qrc       # Qt resource file
├── CMakeLists.txt      # CMake build configuration
└── README.md          # This file
```

## Legal Disclaimer

**IMPORTANT**: This tool is intended for educational purposes and authorized security testing only. Users are responsible for ensuring they have explicit permission to scan target systems. Unauthorized port scanning may violate local laws and network policies.

- Only scan systems you own or have explicit written permission to test
- Respect network policies and terms of service
- Use responsibly and ethically

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Development Status

- ✅ Core TCP scanning functionality
- ✅ Qt-based graphical interface
- ✅ Custom icons and branding
- ✅ CMake build system
- ✅ Continuous integration setup

## Contact

For questions, suggestions, or security concerns, please open an issue on GitHub.

---

**Remember**: Always use this tool responsibly and only on systems you are authorized to test.
