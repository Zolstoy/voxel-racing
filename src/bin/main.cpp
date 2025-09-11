#include <iostream>
#include <voxelracing/voxelracing.hpp>

int
main()
{
    try
    {
        return voxelracing::start();
    } catch (std::exception const& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    } catch (...)
    {
        std::cout << "other exception" << std::endl;
    }
}
