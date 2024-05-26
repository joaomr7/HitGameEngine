#pragma once

#include "../TestFramework.h"
#include "File/StandardConfigurationFile.h"

namespace hit
{
    test_val config_file_test_1()
    {
        auto source = 
        R"(
        shader {
            name: Test
            pass: "Test long 

            string"

            attributes
            {
                position: vec3       
            }

            vertex::
            {
                // code goes here
            }

            fragment::
            {
                // code goes here
            }
        }

        matertial {
            name: Test
            shader: Test

            color: [1.0, 0.8, 0.8, 1.0]
            texture: "My textture.png"
        }
        )";

        auto configuration = config::StandardConfigurationFile(source);

        test_check(configuration.has_configuration());

        test_success();
    }

    test_val config_file_test_2()
    {
        auto source =
            R"(
            shader {
                name: Test
                pass: "Test long string"

                attributes
                {
                    position: vec3       
                }

                vertex::
                {
                    // code goes here
                }

                fragment::
                {
                    // code goes here
                
            }

            matertial {
                name: Test
                shader: Test

                color: [1.0, 0.8, 0.8, 1.0]
                texture: "My texture.png"
            }
            )";

        auto configuration = config::StandardConfigurationFile(source);

        test_check(!configuration.has_configuration());

        test_success();
    }

    void add_config_file_tests(TestSystem& test_system)
    {
        test_system.add_test(get_test(config_file_test_1));
        test_system.add_test(get_test(config_file_test_2));
    }
}