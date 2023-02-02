#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>

class ErrorHandler {
  public:
    void Report(const std::string& file, const unsigned line,
                const std::string& message);
    void Report(const std::string& file, const unsigned line, const size_t col,
                const std::string& message);
};

#endif /* ERROR_HANDLER_H */
