/*!
 * \file       utils.h
 * \brief      Define methods used to manipulate objects.
 * \author     Enrico Fraccaroli
 * \date       23 Agosto 2014
 * \copyright
 *  RadMud (C) Copyright 2014 by Enrico Fraccaroli.
 *  Permission to copy, use, modify, sell and distribute this software is granted
 *  provided this copyright notice appears in all copies. This software is provided
 *  "as is" without express or implied warranty, and with no claim as to its suitability
 *  for any purpose.
 */
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
/*!
 * \brief Return the current timestamp as "Hours:Minute".
 * \return The current timestamp.
 */
std::string GetCurrentTime();

/*!
 * \brief Return the current date.
 * \return The current date.
 */
std::string GetDate();

/*!
 * \brief Provide a way to prompt every important message both in Console and to file.
 * \param log       The message to log.
 */
void LogMessage(std::string log);

/*!
 * \brief Provide a way to prompt an error.
 * \param log The error message to log.
 */
void LogError(std::string log);

/*!
 * \brief It creates a compressed strem of data.
 * \param uncompressed The input non-compressed stream of data.
 * \param compressed   The resulting compressed stream of data.
 */
void DeflateStream(std::vector<uint8_t> & uncompressed, std::vector<uint8_t> & compressed);

/*!
 * \brief It decompress a compressed strem of data.
 * \param compressed   The input compressed stream of data.
 * \param uncompressed The resulting non-compressed stream of data.
 */
void InflateStream(std::vector<uint8_t> & compressed, std::vector<uint8_t> & uncompressed);

#endif
