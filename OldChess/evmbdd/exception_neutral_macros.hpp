#if !defined(EXCEPTION_NEUTRAL_MACROS_HPP)
#define EXCEPTION_NEUTRAL_MACROS_HPP


#include <stdexcept>


#if defined(__cpp_exceptions)
#define BDD_READER_TERMINATE(message) throw std::runtime_error(message)
#else
#define BDD_READER_TERMINATE(message) std::terminate()
#endif


#endif // EXCEPTION_NEUTRAL_MACROS_HPP
