#pragma once

template<typename index_t, typename data_t>
class VAbstract
{
public:

    explicit VAbstract() { }
    explicit VAbstract(const VAbstract&) { } /* Copy */
    explicit VAbstract(VAbstract&&) { } /* Move */

};
