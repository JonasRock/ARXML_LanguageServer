/**
 * @file lspExceptions.hpp
 * @author Jonas Rock
 * @brief custom exceptions
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

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

    struct malformedElementInsertionException : public std::exception
    {
        const char* what() const throw()
        {
            return "Element to be inserted is malformed";
        }
    };

    struct badEntityException : public std::exception
    {
        const char* what() const throw()
        {
            return "Could not parse JSON RPC entity";
        }
    };
}