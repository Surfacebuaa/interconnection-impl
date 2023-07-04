// Copyright 2023 Ant Group Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gflags/gflags.h"

#include "ic_impl/util.h"

#include "interconnection/v2(rfc)/handshake/protocol_family/ecc.pb.h"

DEFINE_string(curve_type, "curve25519", "elliptic curve type");
DEFINE_string(hash_type, "sha_256", "hash type");
DEFINE_string(hash2curve_strategy, "direct_hash_as_point_x",
              "hash to curve strategy");

namespace ic_impl::protocol_family::ecc {

int32_t SuggestedCurveType() {
  return util::GetFlagValue(
      org::interconnection::v2::protocol::CurveType_descriptor(), "CURVE_TYPE_",
      FLAGS_curve_type);
}

int32_t SuggestedHashType() {
  return util::GetFlagValue(
      org::interconnection::v2::protocol::HashType_descriptor(), "HASH_TYPE_",
      FLAGS_hash_type);
}

int32_t SuggestedHash2curveStrategy() {
  return util::GetFlagValue(
      org::interconnection::v2::protocol::HashToCurveStrategy_descriptor(),
      "HASH_TO_CURVE_STRATEGY_", FLAGS_hash2curve_strategy);
}

int32_t SuggestedPointOctetFormat() {
  return org::interconnection::v2::protocol::POINT_OCTET_FORMAT_UNCOMPRESSED;
}

int32_t SuggestedBitLengthAfterTruncated() {
  return -1;  // -1 means disable this optimization (do not truncate)
}

}  // namespace ic_impl::protocol_family::ecc
