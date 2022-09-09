#pragma once

template<typename it, typename dt>
class VAbstract
{
public:
    using index_t = it;
    using data_t = dt;
    explicit VAbstract() { }
    explicit VAbstract(const VAbstract&) { } /* Copy */
    explicit VAbstract(VAbstract&&) { } /* Move */

};
