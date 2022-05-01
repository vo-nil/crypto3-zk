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

#ifndef CRYPTO3_ZK_PLONK_PLACEHOLDER_PREPROCESSOR_HPP
#define CRYPTO3_ZK_PLONK_PLACEHOLDER_PREPROCESSOR_HPP

#include <nil/crypto3/math/algorithms/unity_root.hpp>
#include <nil/crypto3/math/detail/field_utils.hpp>
#include <nil/crypto3/math/polynomial/polynomial.hpp>
#include <nil/crypto3/math/domains/evaluation_domain.hpp>

#include <nil/crypto3/zk/math/permutation.hpp>
#include <nil/crypto3/zk/snark/systems/plonk/placeholder/detail/placeholder_policy.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/copy_constraint.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/table_description.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint.hpp>
#include <nil/crypto3/zk/snark/arithmetization/plonk/detail/column_polynomial.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {

                template<typename FieldType, typename ParamsType>
                class placeholder_public_preprocessor {
                    typedef detail::placeholder_policy<FieldType, ParamsType> policy_type;

                    typedef typename ParamsType::commitment_scheme_public_type commitment_scheme_public_type;

                    using runtime_size_commitment_scheme_type =
                        typename batched_lpc<FieldType, ParamsType::lpc_params, 0, true>;
                    using public_input_commitment_scheme_type =
                        typename batched_lpc<FieldType, ParamsType::lpc_params, ParamsType::public_input_columns>;
                    using constant_commitment_scheme_type =
                        typename batched_lpc<FieldType, ParamsType::lpc_params, ParamsType::constant_columns>;
                    using selector_commitment_scheme_type =
                        typename batched_lpc<FieldType, ParamsType::lpc_params, ParamsType::selector_columns>;
                    using special_commitment_scheme_type =
                        typename batched_lpc<FieldType, ParamsType::lpc_params, 2>;

                public:

                    struct preprocessed_data_type {
                        typedef typename ParamsType::commitment_scheme_public_type
                            commitment_scheme_public_type;

                        struct public_precommitments_type {
                            typename runtime_size_commitment_scheme_type::precommitment_type id_permutation;
                            typename runtime_size_commitment_scheme_type::precommitment_type sigma_permutation;
                            typename public_input_commitment_scheme_type::precommitment_type public_input;
                            typename constant_commitment_scheme_type::precommitment_type constant;
                            typename selector_commitment_scheme_type::precommitment_type selector;
                            typename special_commitment_scheme_type::precommitment_type special_selectors;
                        };

                        struct public_commitments_type { // TODO: verifier needs this data
                            typename runtime_size_commitment_scheme_type::commitment_type id_permutation;
                            typename runtime_size_commitment_scheme_type::commitment_type sigma_permutation;
                            typename public_input_commitment_scheme_type::commitment_type public_input;
                            typename constant_commitment_scheme_type::commitment_type constant;
                            typename selector_commitment_scheme_type::commitment_type selector;
                            typename special_commitment_scheme_type::commitment_type special_selectors;
                        };

                        // both prover and verifier use this data
                        // fields outside of the common_data_type are used by prover
                        struct common_data_type {
                            std::shared_ptr<math::evaluation_domain<FieldType>> basic_domain;

                            math::polynomial<typename FieldType::value_type> Z;
                            math::polynomial<typename FieldType::value_type> lagrange_0;

                            public_commitments_type commitments;

                            std::array<std::vector<int>,
                                typename ParamsType::arithmetization_params::TotalColumns> columns_rotations;

                            std::size_t rows_amount;
                        };

                        plonk_public_polynomial_table<FieldType,
                            typename ParamsType::arithmetization_params> public_polynomial_table;

                        // S_sigma
                        std::vector<math::polynomial<typename FieldType::value_type>> permutation_polynomials;
                        // S_id
                        std::vector<math::polynomial<typename FieldType::value_type>> identity_polynomials;

                        math::polynomial<typename FieldType::value_type> q_last; // TODO: move to common data
                        math::polynomial<typename FieldType::value_type> q_blind;

                        public_precommitments_type precommitments;

                        common_data_type common_data;
                    };

                private:

                    typedef typename preprocessed_data_type::public_precommitments
                        public_precommitments_type;

                    static math::polynomial<typename FieldType::value_type>
                        lagrange_polynomial(std::shared_ptr<math::evaluation_domain<FieldType>> domain,
                                            std::size_t number) {
                        std::vector<std::pair<typename FieldType::value_type, typename FieldType::value_type>>
                            evaluation_points;
                        for (std::size_t i = 0; i < domain->m; i++) {
                            evaluation_points.push_back(std::make_pair(domain->get_domain_element(i),
                                                                       (i != number) ? FieldType::value_type::zero() :
                                                                                       FieldType::value_type::one()));
                        }
                        math::polynomial<typename FieldType::value_type> f =
                            math::lagrange_interpolation(evaluation_points);

                        return f;
                    }

                    struct cycle_representation {
                        typedef std::pair<std::size_t, std::size_t> key_type;

                        std::map<key_type, key_type> _mapping;
                        std::map<key_type, key_type> _aux;
                        std::map<key_type, std::size_t> _sizes;

                        cycle_representation(
                            typename policy_type::constraint_system_type &constraint_system,
                            const plonk_table_description<FieldType, typename ParamsType::arithmetization_params>
                                &table_description) {

                            for (std::size_t i = 0;
                                 i < table_description.table_width() - table_description.selector_columns;
                                 i++) {
                                for (std::size_t j = 0; j < table_description.rows_amount; j++) {
                                    key_type key(i, j);
                                    this->_mapping[key] = key;
                                    this->_aux[key] = key;
                                    this->_sizes[key] = 1;
                                }
                            }

                            std::vector<plonk_copy_constraint<FieldType>> copy_constraints =
                                constraint_system.copy_constraints();
                            for (std::size_t i = 0; i < copy_constraints.size(); i++) {
                                std::size_t x_idx = table_description.global_index(copy_constraints[i].first);
                                key_type x = key_type(x_idx, copy_constraints[i].first.rotation);

                                std::size_t y_idx = table_description.global_index(copy_constraints[i].second);
                                key_type y = key_type(y_idx, copy_constraints[i].second.rotation);
                                this->apply_copy_constraint(x, y);
                            }
                        }

                        void apply_copy_constraint(key_type x, key_type y) {

                            if (!_mapping.count(x)) {
                                _mapping[x] = x;
                                _aux[x] = x;
                                _sizes[x] = 1;
                            }

                            if (!_mapping.count(y)) {
                                _mapping[y] = y;
                                _aux[y] = y;
                                _sizes[y] = 1;
                            }

                            if (_aux[x] != _aux[y]) {
                                key_type &left = x;
                                key_type &right = y;
                                if (_sizes[_aux[left]] < _sizes[_aux[right]]) {
                                    left = y;
                                    right = x;
                                }

                                _sizes[_aux[left]] = _sizes[_aux[left]] + _sizes[_aux[right]];

                                key_type z = _aux[right];
                                key_type exit_condition = _aux[right];

                                do {
                                    _aux[z] = _aux[left];
                                    z = _mapping[z];
                                } while (z != exit_condition);

                                key_type tmp = _mapping[left];
                                _mapping[left] = _mapping[right];
                                _mapping[right] = tmp;
                            }
                        }

                        key_type &operator[](key_type key) {
                            return _mapping[key];
                        }
                    };

                public:
                    static inline std::vector<math::polynomial<typename FieldType::value_type>>
                        identity_polynomials(std::size_t permutation_size, std::size_t table_size,
                                             const typename FieldType::value_type &omega,
                                             const typename FieldType::value_type &delta,
                                             const std::shared_ptr<math::evaluation_domain<FieldType>> &domain) {

                        std::vector<math::polynomial<typename FieldType::value_type>> S_id(permutation_size);

                        for (std::size_t i = 0; i < permutation_size; i++) {
                            std::vector<typename FieldType::value_type> tmp(table_size);
                            for (std::size_t j = 0; j < table_size; j++) {
                                tmp[j] = delta.pow(i) * omega.pow(j);
                            }

                            domain->inverse_fft(tmp);
                            S_id[i] = math::polynomial<typename FieldType::value_type>(tmp);
                        }

                        return S_id;
                    }

                    static inline std::array<std::vector<int>,
                        ParamsType::arithmetization_params::TotalColumns>
                        columns_rotations(
                            typename policy_type::constraint_system_type &constraint_system,
                            const plonk_table_description<FieldType, typename ParamsType::arithmetization_params>
                                &table_description) {

                        std::array<std::vector<int>,
                            ParamsType::arithmetization_params::TotalColumns> result;

                        std::vector<plonk_gate<FieldType, plonk_constraint<FieldType>>> gates = constraint_system.gates();

                        for (std::size_t g_index = 0; g_index < gates.size(); g_index++) {

                            for (std::size_t c_index = 0;
                                c_index < gates[g_index].constraints.size(); c_index++) {
                                
                                for (std::size_t t_index = 0;
                                    t_index < gates[g_index].constraints[c_index].terms.size(); t_index++){
                                    for (std::size_t v_index = 0;
                                        v_index < gates[g_index].constraints[c_index].terms[t_index].vars.size();
                                        v_index++){

                                        if (gates[g_index].constraints[c_index].terms[t_index].vars[v_index].relative){
                                            std::size_t column_index =
                                                table_description.global_index(gates[g_index].constraints[c_index].terms[t_index].vars[v_index]);

                                            int rotation =
                                                gates[g_index].constraints[c_index].terms[t_index].vars[v_index].rotation;

                                            if (std::find(result[column_index].begin(),
                                                    result[column_index].end(), rotation) ==
                                                result[column_index].end()){
                                                result[column_index].push_back(rotation);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        std::vector<plonk_gate<FieldType, plonk_lookup_constraint<FieldType>>> lookup_gates = constraint_system.lookup_gates();

                        for (std::size_t g_index = 0; g_index < lookup_gates.size(); g_index++) {

                            for (std::size_t c_index = 0;
                                c_index < lookup_gates[g_index].constraints.size(); c_index++) {
                                
                                for (std::size_t v_index = 0;
                                    v_index < lookup_gates[g_index].constraints[c_index].lookup_input.size(); v_index++){

                                        if (lookup_gates[g_index].constraints[c_index].lookup_input[v_index].vars[0].relative){
                                            std::size_t column_index =
                                                table_description.global_index(lookup_gates[g_index].constraints[c_index].lookup_input[v_index].vars[0]);

                                            int rotation =
                                                lookup_gates[g_index].constraints[c_index].lookup_input[v_index].vars[0].rotation;

                                            if (std::find(result[column_index].begin(),
                                                    result[column_index].end(), rotation) ==
                                                result[column_index].end()){
                                                result[column_index].push_back(rotation);
                                            }
                                    }
                                }
                            }
                        }

                        for (std::size_t i = 0; i < ParamsType::arithmetization_params::TotalColumns; i++) {
                            if (std::find(result[i].begin(),
                                                    result[i].end(), 0) ==
                                                result[i].end()) {
                                result[i].push_back(0);
                            }
                        }

                        return result;
                    }

                    static inline std::vector<math::polynomial<typename FieldType::value_type>>
                        permutation_polynomials(std::size_t permutation_size, std::size_t table_size,
                                                const typename FieldType::value_type &omega,
                                                const typename FieldType::value_type &delta,
                                                cycle_representation &permutation,
                                                const std::shared_ptr<math::evaluation_domain<FieldType>> &domain) {

                        std::vector<math::polynomial<typename FieldType::value_type>> S_perm(permutation_size);

                        for (std::size_t i = 0; i < permutation_size; i++) {
                            std::vector<typename FieldType::value_type> tmp(table_size);
                            for (std::size_t j = 0; j < table_size; j++) {
                                auto key = std::make_pair(i, j);
                                tmp[j] = delta.pow(permutation[key].first) * omega.pow(permutation[key].second);
                            }

                            domain->inverse_fft(tmp);
                            S_perm[i] = math::polynomial<typename FieldType::value_type>(tmp);
                        }

                        return S_perm;
                    }

                    static inline math::polynomial<typename FieldType::value_type>
                        selector_blind(std::size_t table_size, std::size_t usable_rows,
                                       const std::shared_ptr<math::evaluation_domain<FieldType>> &domain) {

                        std::vector<typename FieldType::value_type> tmp(table_size);
                        for (std::size_t j = 0; j < table_size; j++) {
                            tmp[j] = j > usable_rows ? FieldType::value_type::one() : FieldType::value_type::zero();
                        }

                        domain->inverse_fft(tmp);
                        math::polynomial<typename FieldType::value_type> q_blind(tmp);

                        return q_blind;
                    }

                    static inline math::polynomial<typename FieldType::value_type>
                        selector_last(std::size_t table_size, std::size_t usable_rows,
                                      const std::shared_ptr<math::evaluation_domain<FieldType>> &domain) {

                        std::vector<typename FieldType::value_type> tmp(table_size);
                        for (std::size_t j = 0; j < table_size; j++) {
                            tmp[j] = j == usable_rows ? FieldType::value_type::one() : FieldType::value_type::zero();
                        }

                        domain->inverse_fft(tmp);
                        math::polynomial<typename FieldType::value_type> q_last(tmp);

                        return q_last;
                    }

                    static inline typename preprocessed_data_type::public_precommitments_type precommitments(
                        const plonk_public_polynomial_table<FieldType, typename ParamsType::arithmetization_params>
                            &public_table,
                        std::vector<math::polynomial<typename FieldType::value_type>> &id_perm_polys,
                        std::vector<math::polynomial<typename FieldType::value_type>> &sigma_perm_polys,
                        math::polynomial<typename FieldType::value_type> &q_last,
                        math::polynomial<typename FieldType::value_type> &q_blind,
                        const typename commitment_scheme_public_type::params_type &commitment_params) {

                        typename runtime_size_commitment_scheme_type::precommitment_type id_permutation =
                            runtime_size_commitment_scheme_type::precommit(id_perm_polys, commitment_params.D[0]);

                        typename commitment_scheme_public_type::precommitment_type sigma_permutation =
                            runtime_size_commitment_scheme_type::precommit(sigma_perm_polys, commitment_params.D[0]);

                        typename commitment_scheme_public_type::precommitment_type public_input_precommitment =
                            public_input_commitment_scheme_type::precommit(
                                public_table.public_inputs(), commitment_params.D[0]);

                        typename constant_commitment_scheme_type::precommitment_type constant_precommitment =
                            constant_commitment_scheme_type::precommit(
                                public_table.constants(), commitment_params.D[0]);

                        typename selector_commitment_scheme_type::precommitment_type, selector_precommitment =
                            selector_commitment_scheme_type::precommit(
                                public_table.selectors(), commitment_params.D[0]);

                        typename special_commitment_scheme_type::precommitment_type special_selector_precommitment =
                            special_commitment_scheme_type::precommit({{q_last, q_blind}}, commitment_params.D[0]);

                        return typename preprocessed_data_type::public_precommitments_type {
                            id_permutation,          sigma_permutation,       public_input_precommitment,
                            constant_precommitment, selector_precommitment, special_selector_precommitment};
                    }

                    static inline typename preprocessed_data_type::public_commitments_type
                        commitments(const typename preprocessed_data_type::public_precommitments_type &precommitments) {

                        typename runtime_size_commitment_scheme_type::commitment_type id_permutation =
                            runtime_size_commitment_scheme_type::commit(precommitments.id_permutation);

                        typename runtime_size_commitment_scheme_type::commitment_type sigma_permutation =
                            runtime_size_commitment_scheme_type::commit(precommitments.sigma_permutation);

                        typename public_input_commitment_scheme_type::commitment_type public_input_commitment =
                            public_input_commitment_scheme_type::commit(precommitments.public_input);

                        typename constant_commitment_scheme_type::commitment_type constant_commitment =
                            constant_commitment_scheme_type::commit(precommitments.constant);

                        typename selector_commitment_scheme_type::commitment_type selector_commitment =
                            selector_commitment_scheme_type::commit(precommitments.selector);

                        typename special_commitment_scheme_type::commitment_type special_selector_commitment =
                            special_commitment_scheme_type::commit(precommitments.special_selectors);

                        return typename preprocessed_data_type::public_commitments_type {
                            id_permutation,           sigma_permutation,
                                                        public_input_commitment, constant_commitment,
                                                        selector_commitment,     special_selector_commitment};
                    }

                    static inline typename preprocessed_data_type process(
                        typename policy_type::constraint_system_type &constraint_system,
                        const typename policy_type::variable_assignment_type::public_table_type &public_assignment,
                        const plonk_table_description<FieldType, typename ParamsType::arithmetization_params>
                            &table_description,
                        const typename commitment_scheme_public_type::params_type &commitment_params,
                        std::size_t columns_with_copy_constraints) {

                        std::size_t N_rows = table_description.rows_amount;
                        std::size_t usable_rows = table_description.usable_rows_amount;

                        std::shared_ptr<math::evaluation_domain<FieldType>> basic_domain =
                            math::make_evaluation_domain<FieldType>(N_rows);

                        // TODO: add std::vector<std::size_t> columns_with_copy_constraints;
                        cycle_representation permutation(constraint_system, table_description);

                        std::vector<math::polynomial<typename FieldType::value_type>> _permutation_polynomials =
                            permutation_polynomials(
                                columns_with_copy_constraints, N_rows, basic_domain->get_domain_element(1),
                                policy_type::placeholder_params_type::delta, permutation, basic_domain);

                        std::vector<math::polynomial<typename FieldType::value_type>> _identity_polynomials =
                            identity_polynomials(columns_with_copy_constraints, N_rows,
                                                 basic_domain->get_domain_element(1),
                                                 policy_type::placeholder_params_type::delta, basic_domain);

                        math::polynomial<typename FieldType::value_type> lagrange_0 =
                            lagrange_polynomial(basic_domain, 0);

                        math::polynomial<typename FieldType::value_type> q_last =
                            selector_last(N_rows, usable_rows, basic_domain);
                        math::polynomial<typename FieldType::value_type> q_blind =
                            selector_blind(N_rows, usable_rows, basic_domain);

                        plonk_public_polynomial_table<FieldType, typename ParamsType::arithmetization_params>
                            public_polynomial_table =
                                plonk_public_polynomial_table<FieldType, typename ParamsType::arithmetization_params>(
                                    detail::column_range_polynomials<FieldType>(public_assignment.public_inputs(),
                                                                                basic_domain),
                                    detail::column_range_polynomials<FieldType>(public_assignment.constants(),
                                                                                basic_domain),
                                    detail::column_range_polynomials<FieldType>(public_assignment.selectors(),
                                                                                basic_domain));

                        std::vector<typename FieldType::value_type> Z(N_rows + 1);
                        Z[0] = -FieldType::value_type::one();
                        Z[N_rows] = FieldType::value_type::one();

                        // prepare commitments for short verifier
                        typename preprocessed_data_type::public_precommitments_type public_precommitments =
                            precommitments(public_polynomial_table, _identity_polynomials, _permutation_polynomials,
                                           q_last, q_blind, commitment_params);

                        typename preprocessed_data_type::public_commitments_type public_commitments = commitments(public_precommitments);

                        std::array<std::vector<int>,
                        ParamsType::arithmetization_params::TotalColumns> c_rotations =
                            columns_rotations(constraint_system, table_description);

                        typename preprocessed_data_type::common_data_type common_data {
                            basic_domain, nil::crypto3::math::polynomial<typename FieldType::value_type> {Z},
                            lagrange_0, public_commitments, c_rotations, N_rows};

                        return typename preprocessed_data_type(
                            {public_polynomial_table, _permutation_polynomials, _identity_polynomials, q_last, q_blind,
                             public_precommitments, common_data});
                    }
                };

                template<typename FieldType, typename ParamsType>
                class placeholder_private_preprocessor {
                    using policy_type = detail::placeholder_policy<FieldType, ParamsType>;

                public:

                    struct preprocessed_data_type {

                        std::shared_ptr<math::evaluation_domain<FieldType>> basic_domain;

                        plonk_private_polynomial_table<FieldType,
                            typename ParamsType::arithmetization_params> private_polynomial_table;
                    };

                    static inline typename preprocessed_data_type process(
                        const typename policy_type::constraint_system_type &constraint_system,
                        const typename policy_type::variable_assignment_type::private_table_type &private_assignment,
                        const plonk_table_description<FieldType, typename ParamsType::arithmetization_params>
                            &table_description) {

                        std::size_t N_rows = table_description.rows_amount;

                        std::shared_ptr<math::evaluation_domain<FieldType>> basic_domain =
                            math::make_evaluation_domain<FieldType>(N_rows);

                        plonk_private_polynomial_table<FieldType, typename ParamsType::arithmetization_params>
                            private_polynomial_table =
                                plonk_private_polynomial_table<FieldType, typename ParamsType::arithmetization_params>(
                                    detail::column_range_polynomials<FieldType>(private_assignment.witnesses(),
                                                                                basic_domain));
                        return typename preprocessed_data_type({basic_domain, private_polynomial_table});
                    }
                };

            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ZK_PLONK_PLACEHOLDER_PREPROCESSOR_HPP
