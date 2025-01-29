#pragma once

namespace datasketches {

enum agg_state_type { SKETCH, UNION };

class hll_data 
{
    public:
        hll_data()
            : m_type(SKETCH)
        {
        }

        hll_data(agg_state_type t)
            : m_type(t)
        {
        }

        virtual ~hll_data() = default;

        void set_type(agg_state_type t)
        {
            m_type = t;
        }

        agg_state_type get_type() const
        {
            return m_type;
        }

    private:
        agg_state_type m_type;
};

}

