/**
 * @file lspExceptions.hpp
 * @author Jonas Rock
 * @brief Custom exceptions
 * @version 0.1
 * @date 2020-10-27
 */

#ifndef LSPEXCEPTIONS_H
#define LSPEXCEPTIONS_H

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

    struct multipleDefinitionException : public std::exception
    {
        const char* what() const throw()
        {
            return "Multiple definitions not allowed in this context";
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

    struct badUriException : public std::exception
    {
        const char* what() const throw()
        {
            return "bad URI format";
        }
    };
}

#endif /* LSPEXCEPTIONS_H */