#include "Platform/Platform.h"

#include "Core/Memory.h"
#include "Core/Assert.h"

#include <windows.h>

namespace hit
{
    struct ExternalPackage
    {
        HMODULE package_handle;
    };

    ExternalPackage* Platform::load_external_package(std::string_view package_path)
    {
        HMODULE package_handle = LoadLibrary(package_path.data());

        if(package_handle)
        {
            auto package = (ExternalPackage*)Memory::allocate_memory(sizeof(ExternalPackage), MemoryUsage::Platform);
            hit_assert(package, "Failed to allocate external package!");

            package->package_handle = package_handle;
            return package;
        }

        return nullptr;
    }
    
    bool Platform::unload_external_package(ExternalPackage* package)
    {
        if(!package)
        {
            hit_warning("Attempting to unload an invalid package pointer.");
            return false;
        }

        if(!FreeLibrary(package->package_handle))
        {
            hit_error("Failed to unload package!");
            return false;
        }

        Memory::deallocate_memory((ui8*)package);
        return true;
    }

    PackageFunction Platform::get_package_function(ExternalPackage* package, std::string_view function_name)
    {
        hit_assert(package, "Attempting to load a function from an invalid package pointer!");

        PackageFunction fun_address = (PackageFunction)GetProcAddress(package->package_handle, function_name.data());

        if(!fun_address)
        {
            hit_error("Failed to load package function {}.", function_name);
            return nullptr;
        }

        return fun_address;
    }
}