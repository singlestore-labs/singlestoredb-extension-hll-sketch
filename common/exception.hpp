#ifndef DATASKETCHES_EXCEPTION_HPP_
#define DATASKETCHES_EXCEPTION_HPP_

#define THROW_EXCEPTION(msg,code,ex) { std::cerr << msg << std::endl; exit(code); }

#define THROW_INVALID_ARG(msg) THROW_EXCEPTION(msg, 1, std::invalid_argument)
#define THROW_OUT_OF_RANGE(msg) THROW_EXCEPTION(msg, 2, std::out_of_range)
#define THROW_LOGIC_ERR(msg) THROW_EXCEPTION(msg, 3, std::logic_error)
#define THROW_RUNTIME_ERR(msg) THROW_EXCEPTION(msg, 4, std::runtime_error)

#endif // DATASKETCHES_EXCEPTION_HPP_