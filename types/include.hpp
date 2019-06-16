//
// Created by cerussite on 19/06/15.
//

#ifndef MAKE_INCLUDE_HPP
#define MAKE_INCLUDE_HPP

#include <string>

namespace make {
    class Include {
    private:
        bool _errorIgnored;
        std::string _file;

    public:
        Include(bool errorIgnored, std::string file)
            : _errorIgnored(errorIgnored)
            , _file(std::move(file)) {}
    };

} // namespace make

#endif // MAKE_INCLUDE_HPP
