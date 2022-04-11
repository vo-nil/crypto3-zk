//---------------------------------------------------------------------------//
// Copyright (c) 2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2022 Ilia Shirobokov <i.shirobokov@nil.foundation>
// Copyright (c) 2022 Alisa Cherniaeva <a.cherniaeva@nil.foundation>
//
// MIT License
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
//---------------------------------------------------------------------------//
// @file Declaration of interfaces for:
//
// - a PLONK gate,
// - a PLONK variable assignment, and
// - a PLONK constraint system.
//
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_PLONK_CONSTRAINT_SYSTEM_HPP
#define CRYPTO3_ZK_PLONK_CONSTRAINT_SYSTEM_HPP

#include <cstdlib>
#include <vector>

#include <nil/crypto3/zk/snark/arithmetization/plonk/gate.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/copy_constraint.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/lookup_constraint.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {

                /************************* PLONK constraint system ****************************/

                template<typename FieldType, typename ArithmetizationParams>
                struct plonk_constraint_system {

                protected:
                    std::vector<plonk_gate<FieldType, plonk_constraint<FieldType>>> _gates;
                    std::vector<plonk_copy_constraint<FieldType>> _copy_constraints;
                    std::vector<plonk_gate<FieldType, plonk_lookup_constraint<FieldType>>> _lookup_gates;

                public:

                    typedef FieldType field_type;

                    plonk_constraint_system(){}

                    plonk_constraint_system(
                        std::vector<plonk_gate<FieldType, plonk_constraint<FieldType>>> &gates,
                        std::vector<plonk_copy_constraint<FieldType>> &copy_constraints,
                        std::vector<plonk_gate<FieldType, plonk_lookup_constraint<FieldType>>>  &lookup_gates):
                        _gates(gates), _copy_constraints(copy_constraints), _lookup_gates(lookup_gates){
                    }

                    std::size_t num_gates() const {
                        return _gates.size();
                    }

                    // bool
                    //     is_satisfied(plonk_variable_assignment<FieldType, WitnessColumns> full_variable_assignment)
                    //     const {

                    //     for (std::size_t c = 0; c < constraints.size(); ++c) {
                    //         if (!constraints[c].a.evaluate(full_variable_assignment).is_zero()) {
                    //             return false;
                    //         }
                    //     }

                    //     return true;
                    // }

                    std::vector<plonk_gate<FieldType, plonk_constraint<FieldType>>> gates() const {
                        return _gates;
                    }

                    std::vector<plonk_copy_constraint<FieldType>> copy_constraints() const {
                        return _copy_constraints;
                    }

                    std::vector<plonk_gate<FieldType, plonk_lookup_constraint<FieldType>>> lookup_gates() const {
                        return _lookup_gates;
                    }
                };
            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ZK_PLONK_CONSTRAINT_SYSTEM_HPP
