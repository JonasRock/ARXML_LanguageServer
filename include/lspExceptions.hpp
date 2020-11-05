/**
 * @file lspExceptions.hpp
 * @author Jonas Rock
 * @brief Custom exceptions
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#ifndef LSPEXCEPTIONS_H
#define LSPEXCEPTIONS_H

#include <exception>

namespace lsp
{
    /**
     * @brief Cannot find requested element in shortnameStorage
     * 
     */
    struct elementNotFoundException : public std::exception
    {
        const char* what() const throw()
        {
            return "Could not find element";
        }
    };

    /**
     * @brief Element to be inserted into shortnameStorage is malformed
     * 
     */
    struct malformedElementInsertionException : public std::exception
    {
        const char* what() const throw()
        {
            return "Element to be inserted is malformed";
        }
    };

    /**
     * @brief Message is not correct or not registered
     * 
     */
    struct badEntityException : public std::exception
    {
        const char* what() const throw()
        {
            return "Could not parse JSON RPC entity";
        }
    };
}

#endif /* LSPEXCEPTIONS_H */