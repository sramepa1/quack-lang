#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

class NoSuchFieldException : public std::runtime_error {
public:
	NoSuchFieldException(const std::string & msg) : std::runtime_error(msg) {}
};

class NoSuchMethodException : public std::runtime_error {
public:
	NoSuchMethodException(const std::string & msg) : std::runtime_error(msg) {}
};

class NullReferenceException : public std::runtime_error {
public:
	NullReferenceException(const std::string & msg) : std::runtime_error(msg) {}
};

#endif // EXCEPTIONS_H
