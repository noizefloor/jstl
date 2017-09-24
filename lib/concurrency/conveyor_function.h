#pragma once

// MIT License
//
// Copyright (c) 2017 Jan Schwers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <type_traits>

#include "internal/conveyor_forwarder.h"
#include "internal/conveyor.h"
#include "internal/conveyor_assertions.h"

/**
 * jstd
 * @brief Jan Schwers Template Library namespace
 */
namespace jstd
{
    /**
     * @addtogroup concurrency
     * @{
     */

    /**
     * @brief Runs multiple callables on separated threads, while managing the synchronization of forwarded data
     * between the threads.
     *
     * The function takes a variadic number of callables (at least two). Every callable can forward data
     * (variables, objects...) to the following callable, by pushing it to the conveyor_forwarder.
     * The following callable will be called on a different thread and will receive the data as a rvalue reference.
     * The synchronization of the forwarded data between the
     * threads is handled by conveyor_function.
     *
     *
     * @tparam Callable_0 Type of the first callable.
     * <ul>
     *     <li>
     *         This callable is called the producer.<br>
     *         The signature of a producer callable is expected to look like:
     *         @code void(conveyor_forwarder<Target_type>&) @endcode
     *     </li>
     * </ul>
     * @tparam Callable_1 Type of the second callable.
     * <ul>
     *     <li>
     *         If this callable is the last, it is called the consumer.<br>
     *         The signature of a consumer callable is expected to look like:
     *         @code void(Source_type&&) @endcode
     *     </li>
     *     <li>
     *         If this callable is not the last, it is called a converter.<br>
     *         The signature of a converter callable is expected to look like:
     *         @code void(Source_type&&, conveyor_forwarder<Target_type>&) @endcode
     *     </li>
     * </ul>
     * @tparam Callable_N Parameter pack of more callables.
     * <ul>
     *     <li>
     *         All, but the last callable must meet the requirements of a
     *         converter.
     *     </li>
     *     <li>
     *         The last callable must meet the requirements of a consumer.
     *     </li>
     * </ul>
     * @param callable_0 The first callable.<ul><li>Must meet all requirements of a producer.</li></ul>
     * @param callable_1 The second callable.
     * <ul>
     *     <li>If this is the last callable, it must meet the requirements of a consumer.</li>
     *     <li>If this is not the last callable, it must meet the requirements of a converter.</li>
     * </ul>
     * @param callable_n Parameter pack of more callables.
     * <ul><li>If callable_1 meets the requirements of a consumer, the parameter pack needs to be empty.</li></ul>
     */
    template <typename Callable_0, typename Callable_1, typename... Callable_N>
    void conveyor_function(Callable_0&& callable_0, Callable_1&& callable_1, Callable_N&&... callable_n)
    {
        internal::assert_signature<Callable_0, Callable_1, Callable_N...>();

        auto&& conveyor = internal::make_conveyor(std::forward<Callable_1>(callable_1),
                                                  std::forward<Callable_N>(callable_n)...);
        try
        {
            callable_0(conveyor->getForwarder());
        }
        catch (...)
        {
            conveyor->finish();
            throw;
        }

        conveyor->finish();
        conveyor->checkForError();
    };

    /**@}*/

} // jstd


