// ErrorHandler.hpp
#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include "qtheaders.hpp"
#include <string>
#include <functional>
#include <iostream>

namespace DiveComputer {

// Error severity levels
enum class ErrorSeverity {
    INFO,       // Informational message
    WARNING,    // Warning that doesn't prevent operation
    ERROR,      // Error that prevents specific operation
    CRITICAL    // Critical error that may require application restart
};

class ErrorHandler {
public:
    // Display error dialog with appropriate styling based on severity
    static void showErrorDialog(const QString& title, const QString& message, 
                               ErrorSeverity severity = ErrorSeverity::ERROR) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(title);
        msgBox.setText(message);
        
        switch (severity) {
            case ErrorSeverity::INFO:
                msgBox.setIcon(QMessageBox::Information);
                break;
            case ErrorSeverity::WARNING:
                msgBox.setIcon(QMessageBox::Warning);
                break;
            case ErrorSeverity::ERROR:
                msgBox.setIcon(QMessageBox::Critical);
                break;
            case ErrorSeverity::CRITICAL:
                msgBox.setIcon(QMessageBox::Critical);
                break;
        }
        
        msgBox.exec();
    }
    
    // Log error to console
    static void logError(const std::string& context, const std::string& message, 
                        ErrorSeverity severity = ErrorSeverity::ERROR) {
        std::string severityStr;
        switch (severity) {
            case ErrorSeverity::INFO:
                severityStr = "INFO";
                break;
            case ErrorSeverity::WARNING:
                severityStr = "WARNING";
                break;
            case ErrorSeverity::ERROR:
                severityStr = "ERROR";
                break;
            case ErrorSeverity::CRITICAL:
                severityStr = "CRITICAL";
                break;
        }
        
        std::cerr << "[" << severityStr << "] " << context << ": " << message << std::endl;
    }
    
    // Try operation with error handling
    template<typename Func>
    static bool tryOperation(Func operation, const std::string& context, 
                        const QString& errorTitle,
                        bool showDialog = true) {
        try {
            operation();
            return true;
        }
        catch (const std::exception& e) {
            std::string errorMsg = e.what();
            logError(context, errorMsg);
            
            if (showDialog) {
                // Explicitly call the static method with correct arguments
                ErrorHandler::showErrorDialog(errorTitle, QString::fromStdString(errorMsg));
            }
            return false;
        }
        catch (...) {
            std::string errorMsg = "Unknown error occurred";
            logError(context, errorMsg);
            
            if (showDialog) {
                // Explicitly call the static method with correct arguments
                ErrorHandler::showErrorDialog(errorTitle, QString::fromStdString(errorMsg));
            }
            return false;
        }
    }
    
    // Try file operation with specific error handling
    static bool tryFileOperation(std::function<void()> operation, 
                               const std::string& filePath,
                               const QString& errorTitle,
                               bool showErrorDialog = true) {
        try {
            operation();
            return true;
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::string errorMsg = "Filesystem error: " + std::string(e.what());
            logError("File operation on " + filePath, errorMsg);
            
            if (showErrorDialog) {
                ErrorHandler::showErrorDialog(errorTitle, 
                               QString("Error accessing file: %1\n\nDetails: %2")
                               .arg(QString::fromStdString(filePath))
                               .arg(QString::fromStdString(e.what())));
            }
            return false;
        }
        catch (const std::ios_base::failure& e) {
            std::string errorMsg = "I/O error: " + std::string(e.what());
            logError("File operation on " + filePath, errorMsg);
            
            if (showErrorDialog) {
                ErrorHandler::showErrorDialog(errorTitle, 
                               QString("Error reading/writing file: %1\n\nDetails: %2")
                               .arg(QString::fromStdString(filePath))
                               .arg(QString::fromStdString(e.what())));
            }
            return false;
        }
        catch (const std::exception& e) {
            std::string errorMsg = e.what();
            logError("File operation on " + filePath, errorMsg);
            
            if (showErrorDialog) {
                ErrorHandler::showErrorDialog(errorTitle, 
                               QString("Error with file: %1\n\nDetails: %2")
                               .arg(QString::fromStdString(filePath))
                               .arg(QString::fromStdString(e.what())));
            }
            return false;
        }
        catch (...) {
            std::string errorMsg = "Unknown error occurred";
            logError("File operation on " + filePath, errorMsg);
            
            if (showErrorDialog) {
                ErrorHandler::showErrorDialog(errorTitle, 
                               QString("Unknown error with file: %1")
                               .arg(QString::fromStdString(filePath)));
            }
            return false;
        }
    }
    
    // Validate numeric input with bounds checking
    static bool validateNumericInput(const QString& input, double& value,
                                   double minValue, double maxValue,
                                   const QString& fieldName,
                                   bool showErrorDialog = true) {
        bool ok;
        value = input.toDouble(&ok);
        
        if (!ok) {
            if (showErrorDialog) {
                ErrorHandler::showErrorDialog(QString("Invalid Input"), 
                               QString("'%1' is not a valid number for %2.")
                               .arg(input)
                               .arg(fieldName),
                               ErrorSeverity::WARNING);
            }
            return false;
        }
        
        if (value < minValue || value > maxValue) {
            if (showErrorDialog) {
                ErrorHandler::showErrorDialog(QString("Out of Range"), 
                               QString("Value for %1 must be between %2 and %3.")
                               .arg(fieldName)
                               .arg(minValue)
                               .arg(maxValue),
                               ErrorSeverity::WARNING);
            }
            return false;
        }
        
        return true;
    }
};

} // namespace DiveComputer

#endif // ERROR_HANDLER_HPP