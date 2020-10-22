#include <exception>

namespace lsp
{
    struct elementNotFoundException : public std::exception
    {
        const char* what() const throw()
        {
            return "Could not find element";
        }
    };
}