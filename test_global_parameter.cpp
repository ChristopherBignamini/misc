#include "stencil-composition/stencil-composition.hpp"
#include "stencil-composition/global_accessor.hpp"
#include "stencil-composition/backend.hpp"

typedef gridtools::interval<gridtools::level<0,-1>, gridtools::level<1,-1> > x_interval;
typedef gridtools::interval<gridtools::level<0,-2>, gridtools::level<1,1> > axis;

#ifdef __CUDACC__
#define BACKEND_ARCH gridtools::enumtype::Cuda
#define BACKEND_STRATEGY gridtools::enumtype::Block
#else
#ifdef BACKEND_BLOCK
#define BACKEND_ARCH gridtools::enumtype::Host
#define BACKEND_STRATEGY gridtools::enumtype::Block
#else
#define BACKEND_ARCH gridtools::enumtype::Host
#define BACKEND_STRATEGY gridtools::enumtype::Naive
#endif
#endif

using BACKEND = gridtools::backend<BACKEND_ARCH, gridtools::enumtype::GRIDBACKEND, BACKEND_STRATEGY >;

template <typename MetaData>
using storage_t = gridtools::storage_traits< BACKEND_ARCH >::data_store_t< float, MetaData >;

template<unsigned int ID, unsigned int DIM >
using storage_info_t = gridtools::storage_traits< BACKEND_ARCH >::storage_info_t< ID, DIM >;

using namespace gridtools;


struct global_par_storage {

    global_par_storage() {}

    GT_FUNCTION
    double operator()() const { return 10.; }
};

/*

struct global_par_storage_ext {

    global_par_storage_ext() {}

    GT_FUNCTION
    double operator()(unsigned int i) const { return 10.; }
};
*/

struct functor {

    typedef global_accessor< 0, enumtype::in > g_acc_t;
    typedef accessor< 1, enumtype::inout, extent<>, 3 > acc;

    typedef boost::mpl::vector< g_acc_t, acc > arg_list;

    template < typename Evaluation >
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {

        g_acc_t g_acc;

        eval(acc()) += eval(g_acc());
    }
};

/*

struct functor_ext {

    typedef global_accessor< 0, enumtype::in > g_acc_t;
    typedef accessor< 1, enumtype::inout, extent<>, 3 > acc;

    typedef boost::mpl::vector< g_acc_t, acc > arg_list;

    template < typename Evaluation >
    GT_FUNCTION static void Do(Evaluation &eval, x_interval) {

        g_acc_t g_acc;

        eval(acc()) += eval(g_acc(0));
    }
};

*/


int main() {


    constexpr unsigned int d1=5;
    constexpr unsigned int d2=5;
    constexpr unsigned int d3=5;

    global_par_storage gp_stor_1;
    global_par_storage gp_stor_2;
    auto gp_stor_1_gp = BACKEND::make_global_parameter(gp_stor_1);
    auto gp_stor_2_gp = BACKEND::make_global_parameter(gp_stor_2);

    using storage_info_t=storage_info_t< __COUNTER__, 3 >;
    using storage_type=storage_t< storage_info_t >;
    storage_info_t s_(d1,d2,d3);
    storage_type s(s_, 0.e0, "s");

    auto grid=gridtools::grid<axis>({0, 0, 0, d1-1, d1},
        {0, 0, 0, d2-1, d2});
    grid.value_list[0] = 0;
    grid.value_list[1] = d3-1;


    typedef gridtools::arg<0, decltype(gp_stor_1_gp)> p_gp_stor_1;
    typedef gridtools::arg<1, decltype(gp_stor_2_gp)> p_gp_stor_2;
    typedef gridtools::arg<2, storage_type > p_storage;
    typedef boost::mpl::vector<p_gp_stor_1, p_gp_stor_2, p_storage> arg_list;


    gridtools::aggregator_type<arg_list> domain((p_gp_stor_1() = gp_stor_1_gp),
                                         (p_gp_stor_2() = gp_stor_2_gp),
                                         (p_storage() = s));


    auto compute=::gridtools::make_computation<BACKEND>(domain,
							grid,
							gridtools::make_multistage(gridtools::enumtype::execute<gridtools::enumtype::forward>(),
                                                       gridtools::make_stage<functor>(p_gp_stor_1(),p_storage()),
                                                       gridtools::make_stage<functor>(p_gp_stor_2(),p_storage())
										   ));


    compute->ready();
    compute->steady();
    compute->run();
    compute->finalize();

}
