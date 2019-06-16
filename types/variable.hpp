//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_VARIABLE_HPP
#define MAKE_VARIABLE_HPP

#include <string>
#include <vector>

namespace make {
    class Variable {
    public:
        enum class Type {
            Simple,
            Recursive,
            Optional,
            Append,
        };

    private:
        Type _type;
        std::string _lhs;
        std::vector<std::string> _rhs;

    public:
        Variable(Type type, std::string lhs, std::vector<std::string> rhs)
            : _type(type)
            , _lhs(std::move(lhs))
            , _rhs(std::move(rhs)) {}

    public:
        Type type() const noexcept { return _type; }

        const std::string &lhs() const { return _lhs; }
        const std::string &name() const { return lhs(); }

        const std::vector<std::string> &rhs() const { return _rhs; }
        const std::vector<std::string> &value() const { return rhs(); }
    };
} // namespace make

#endif // MAKE_VARIABLE_HPP
