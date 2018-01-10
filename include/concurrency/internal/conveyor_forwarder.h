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


namespace jstd
{
    template <typename T>
    struct is_copy_move_constructible
    {
        static const bool value = std::is_copy_constructible<T>::value && std::is_move_constructible<T>::value;
    };

    template<typename Target_type, typename Enable = void>
    class conveyor_forwarder;

    /**
     * @addtogroup concurrency
     * @{
     */

    /**
     * @brief Virtual class that gets passed to a producer or consumer callable as a parameter.
     *
     * This classed can be used to push data to the following callable that is running on a separated thread.
     * test
     * @tparam Target_type The Type of the data that should be forwarded to the next callable.
     */
    template<typename Target_type>
    class conveyor_forwarder<Target_type, typename std::enable_if<is_copy_move_constructible<Target_type>::value>::type>
    {
    public:
        virtual ~conveyor_forwarder() = default;

        /**@{*/

        /**
         * @brief Pushes a value to the following forwarder.
         * @param forwardValue Rvalue reference to a value that should be forwarded to the next callable that is
         * running on a separated thread.
         */
        virtual void push(Target_type&& forwardValue) = 0;

        /**
         * @brief Pushes a value to the following forwarder.
         * @param forwardValue Lvalue reference to a value that should be forwarded to the next callable that is
         * running on a separated thread.
         * @warning As this method takes a constant lvalue reference a copy of the value needs to be made.
         * Whenever possible the value should be forwarded as rvalue reference to get better performance.
         */
        void push(const Target_type& forwardValue)
        {
            push(Target_type(forwardValue));
        }

        /**@}*/
    };

    /**@}*/

    template<typename T>
    class conveyor_forwarder<T, typename std::enable_if<!std::is_copy_constructible<T>::value >::type>
    {
    public:
        virtual ~conveyor_forwarder() = default;

        virtual void push(T&& forwardValue) = 0;
    };

} // jstd