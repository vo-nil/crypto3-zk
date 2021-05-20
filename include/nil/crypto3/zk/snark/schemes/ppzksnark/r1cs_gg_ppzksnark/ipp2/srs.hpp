//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2020 Ilias Khairullin <ilias@nil.foundation>
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

#ifndef CRYPTO3_R1CS_GG_PPZKSNARK_AGGREGATE_IPP2_SRS_HPP
#define CRYPTO3_R1CS_GG_PPZKSNARK_AGGREGATE_IPP2_SRS_HPP

#include <memory>
#include <vector>
#include <tuple>

#include <nil/crypto3/zk/snark/schemes/ppzksnark/r1cs_gg_ppzksnark/ipp2/commitment.hpp>
#include <nil/crypto3/zk/snark/schemes/ppzksnark/r1cs_gg_ppzksnark/ipp2/multiscalar.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {
                /// ProverSRS is the specialized SRS version for the prover for a specific number of proofs to
                /// aggregate. It contains as well the commitment keys for this specific size.
                /// Note the size must be a power of two for the moment - if it is not, padding must be
                /// applied.
                template<typename CurveType>
                struct r1cs_gg_ppzksnark_proving_srs {
                    typedef CurveType curve_type;
                    typedef r1cs_gg_ppzksnark_ipp2_commitment<CurveType> commitment_type;
                    typedef typename commitment_type::vkey_type vkey_type;
                    typedef typename commitment_type::wkey_type wkey_type;

                    /// Returns true if commitment keys have the exact required length.
                    /// It is necessary for the IPP scheme to work that commitment
                    /// key have the exact same number of arguments as the number of proofs to
                    /// aggregate.
                    bool has_correct_len(std::size_t n) {
                        return vkey.has_correct_len(n) && wkey.has_correct_len(n);
                    }

                    /// number of proofs to aggregate
                    std::size_t n;
                    /// $\{g^a^i\}_{i=0}^{2n-1}$ where n is the number of proofs to be aggregated
                    /// We take all powers instead of only ones from n -> 2n-1 (w commitment key
                    /// is formed from these powers) since the prover will create a shifted
                    /// polynomial of degree 2n-1 when doing the KZG opening proof.
                    multiscalar_precomp_owned<typename CurveType::g1_type> g_alpha_powers_table;
                    /// $\{h^a^i\}_{i=0}^{n-1}$ - here we don't need to go to 2n-1 since v
                    /// commitment key only goes up to n-1 exponent.
                    multiscalar_precomp_owned<typename CurveType::g2_type> h_alpha_powers_table;
                    /// $\{g^b^i\}_{i=0}^{2n-1}$
                    multiscalar_precomp_owned<typename CurveType::g1_type> g_beta_powers_table;
                    /// $\{h^b^i\}_{i=0}^{n-1}$
                    multiscalar_precomp_owned<typename CurveType::g2_type> h_beta_powers_table;
                    /// commitment key using in MIPP and TIPP
                    vkey_type vkey;
                    /// commitment key using in TIPP
                    wkey_type wkey;
                };

                /// Contains the necessary elements to verify an aggregated Groth16 proof; it is of fixed size
                /// regardless of the number of proofs aggregated. However, a verifier SRS will be determined by
                /// the number of proofs being aggregated.
                template<typename CurveType>
                struct r1cs_gg_ppzksnark_verifying_srs {
                    typedef CurveType curve_type;

                    std::size_t n;
                    typename CurveType::g1_type::value_type g;
                    typename CurveType::g2_type::value_type h;
                    typename CurveType::g1_type::value_type g_alpha;
                    typename CurveType::g1_type::value_type g_beta;
                    typename CurveType::g2_type::value_type h_alpha;
                    typename CurveType::g2_type::value_type h_beta;
                };

                /// It contains the maximum number of raw elements of the SRS needed to aggregate and verify
                /// Groth16 proofs. One can derive specialized prover and verifier key for _specific_ size of
                /// aggregations by calling `srs.specialize(n)`. The specialized prover key also contains
                /// precomputed tables that drastically increase prover's performance.
                /// This GenericSRS is usually formed from the transcript of two distinct power of taus ceremony
                /// ,in other words from two distinct Groth16 CRS.
                /// See [there](https://github.com/nikkolasg/taupipp) a way on how to generate this GenesisSRS.
                template<typename CurveType>
                struct r1cs_gg_pp_zksnark_srs {
                    typedef CurveType curve_type;
                    typedef typename curve_type::g1_type g1_type;
                    typedef typename curve_type::g2_type g2_type;
                    typedef typename g1_type::value_type g1_value_type;
                    typedef typename g2_type::value_type g2_value_type;

                    /// $\{g^a^i\}_{i=0}^{N}$ where N is the smallest size of the two Groth16 CRS.
                    std::vector<g1_value_type> g_alpha_powers;
                    /// $\{h^a^i\}_{i=0}^{N}$ where N is the smallest size of the two Groth16 CRS.
                    std::vector<g2_value_type> h_alpha_powers;
                    /// $\{g^b^i\}_{i=n}^{N}$ where N is the smallest size of the two Groth16 CRS.
                    std::vector<g1_value_type> g_beta_powers;
                    /// $\{h^b^i\}_{i=0}^{N}$ where N is the smallest size of the two Groth16 CRS.
                    std::vector<g2_value_type> h_beta_powers;

                    /// Maximum size of the generic SRS constructed from Filecoin and Zcash power of
                    /// taus.
                    ///
                    /// https://github.com/nikkolasg/taupipp/blob/baca1426266bf39416c45303e35c966d69f4f8b4/src/bin/assemble.rs#L12
                    static constexpr std::size_t MAX_SRS_SIZE = (2 << 19) + 1;

                    /// specializes returns the prover and verifier SRS for a specific number of
                    /// proofs to aggregate. The number of proofs MUST BE a power of two, it
                    /// panics otherwise. The number of proofs must be inferior to half of the
                    /// size of the generic srs otherwise it panics.
                    template<typename CurveType>
                    std::pair<r1cs_gg_ppzksnark_proving_srs<CurveType>, r1cs_gg_ppzksnark_verifying_srs<CurveType>>
                        specialize(std::size_t num_proofs) {
                        BOOST_ASSERT((num_proofs & (num_proofs - 1)) == 0);

                        std::size_t tn = 2 * num_proofs;    // size of the CRS we need
                        BOOST_ASSERT(g_alpha_powers.size() >= tn);
                        BOOST_ASSERT(h_alpha_powers.size() >= tn);
                        BOOST_ASSERT(g_beta_powers.size() >= tn);
                        BOOST_ASSERT(h_beta_powers.size() >= tn);

                        std::size_t n = num_proofs;
                        // when doing the KZG opening we need _all_ coefficients from 0
                        // to 2n-1 because the polynomial is of degree 2n-1.
                        std::size_t g_low = 0;
                        std::size_t g_up = tn;
                        std::size_t h_low = 0;
                        std::size_t h_up = h_low + n;
                        multiscalar_precomp_owned<g1_type> g_alpha_powers_table =
                            precompute_fixed_window(g_alpha_powers.begin() + g_low, g_alpha_powers.begin() + g_up,
                                                    multiscalar_precomp_owned<g1_type>::WINDOW_SIZE);
                        multiscalar_precomp_owned<g1_type> g_beta_powers_table =
                            precompute_fixed_window(g_beta_powers.begin() + g_low, g_beta_powers.begin() + g_up,
                                                    multiscalar_precomp_owned<g1_type>::WINDOW_SIZE);
                        multiscalar_precomp_owned<g2_type> h_alpha_powers_table =
                            precompute_fixed_window(h_alpha_powers.begin() + h_low, h_alpha_powers.begin() + h_up,
                                                    multiscalar_precomp_owned<g2_type>::WINDOW_SIZE);
                        multiscalar_precomp_owned<g2_type> h_beta_powers_table =
                            precompute_fixed_window(h_beta_powers.begin() + h_low, h_beta_powers.begin() + h_up,
                                                    multiscalar_precomp_owned<g2_type>::WINDOW_SIZE);
                        std::vector<typename CurveType::g2_type::value_type> v1 = {h_alpha_powers.begin() + h_low,
                                                                                   h_alpha_powers.begin() + h_up};
                        std::vector<typename CurveType::g2_type::value_type> v2 = {h_beta_powers.begin() + h_low,
                                                                                   h_beta_powers.begin() + h_up};
                        typename r1cs_gg_pp_zksnark_srs<CurveType>::vkey_type vkey = {v1, v2};
                        BOOST_ASSERT(vkey.has_correct_len(n));
                        // however, here we only need the "right" shifted bases for the
                        // commitment scheme.
                        std::vector<typename CurveType::g1_type::value_type> w1 = {g_alpha_powers.begin() + n,
                                                                                   g_alpha_powers.begin() + g_up};
                        std::vector<typename CurveType::g1_type::value_type> w2 = {g_beta_powers.begin() + n,
                                                                                   g_beta_powers.begin() + g_up};
                        typename r1cs_gg_pp_zksnark_srs<CurveType>::wkey_type wkey = {w1, w2};
                        BOOST_ASSERT(wkey.has_correct_len(n));

                        r1cs_gg_ppzksnark_proving_srs<CurveType> pk = {n,
                                                                       g_alpha_powers_table,
                                                                       h_alpha_powers_table,
                                                                       g_beta_powers_table,
                                                                       h_beta_powers_table,
                                                                       vkey,
                                                                       wkey};
                        r1cs_gg_ppzksnark_verifying_srs<CurveType> vk = {n,
                                                                         g_alpha_powers[0],
                                                                         h_alpha_powers[0],
                                                                         g_alpha_powers[1],
                                                                         g_beta_powers[1],
                                                                         h_alpha_powers[1],
                                                                         h_beta_powers[1]};
                        return std::make_pair(pk, vk);
                    }
                };

            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_R1CS_GG_PPZKSNARK_AGGREGATE_IPP2_SRS_HPP
