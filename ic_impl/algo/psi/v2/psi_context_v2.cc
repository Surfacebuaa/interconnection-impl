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

#include "ic_impl/algo/psi/v2/psi_context_v2.h"

#include "absl/strings/str_split.h"
#include "gflags/gflags.h"
#include "psi/legacy/bucket_psi.h"
#include "psi/utils/csv_checker.h"

#include "ic_impl/protocol_family/ecc/ecc.h"
#include "ic_impl/util.h"

#include "interconnection/handshake/protocol_family/ecc.pb.h"

DEFINE_string(in_path, "data.csv", "psi data in file path");
DEFINE_string(field_names, "id", "field names");
DEFINE_string(out_path, "", "psi out file path");

DEFINE_int32(result_to_rank, -1, "which rank gets the result");

namespace ic_impl::algo::psi::v2 {

namespace {

std::string GetPsiInputFileName() {
  return util::GetInputFileName(FLAGS_in_path);
}

std::string GetPsiOutputFileName() {
  return util::GetOutputFileName(FLAGS_out_path);
}

std::string GetPsiInputFileFieldNames() {
  return util::GetParamEnv("field_names", FLAGS_field_names);
}

int32_t SuggestedResultToRank() {
  return util::GetParamEnv("result_to_rank", FLAGS_result_to_rank);
}

}  // namespace

using org::interconnection::v2::protocol::CURVE_TYPE_CURVE25519;
using org::interconnection::v2::protocol::CURVE_TYPE_SM2;
using org::interconnection::v2::protocol::
    HASH_TO_CURVE_STRATEGY_DIRECT_HASH_AS_POINT_X;
using org::interconnection::v2::protocol::HASH_TO_CURVE_STRATEGY_TRY_AND_REHASH;
using org::interconnection::v2::protocol::HASH_TYPE_SHA_256;
using org::interconnection::v2::protocol::POINT_OCTET_FORMAT_UNCOMPRESSED;
using org::interconnection::v2::protocol::POINT_OCTET_FORMAT_X962_COMPRESSED;

std::shared_ptr<EcdhPsiContext> CreateEcdhPsiContext(
    std::shared_ptr<IcContext> ic_context) {
  auto ctx = std::make_shared<EcdhPsiContext>();
  ctx->curve_type = protocol_family::ecc::SuggestedCurveType();
  ctx->hash_type = protocol_family::ecc::SuggestedHashType();
  ctx->hash_to_curve_strategy =
      protocol_family::ecc::SuggestedHash2curveStrategy();
  ctx->point_octet_format = protocol_family::ecc::SuggestedPointOctetFormat();
  ctx->bit_length_after_truncated =
      protocol_family::ecc::SuggestedBitLengthAfterTruncated();
  ctx->result_to_rank = SuggestedResultToRank();  // TODO: check

  ctx->ic_ctx = std::move(ic_context);

  return ctx;
}

std::unique_ptr<::psi::BucketPsi> CreateBucketPsi(const EcdhPsiContext &ctx) {
  ::psi::BucketPsiConfig config;
  config.mutable_input_params()->set_path(GetPsiInputFileName());
  auto field_list = absl::StrSplit(GetPsiInputFileFieldNames(), ',');
  config.mutable_input_params()->mutable_select_fields()->Add(
      field_list.begin(), field_list.end());
  config.mutable_input_params()->set_precheck(false);
  config.mutable_output_params()->set_path(GetPsiOutputFileName());
  config.mutable_output_params()->set_need_sort(false);

  config.set_psi_type(::psi::PsiType::ECDH_PSI_2PC);

  if (ctx.result_to_rank == -1) {
    config.set_broadcast_result(true);
  } else {
    config.set_broadcast_result(false);
    config.set_receiver_rank(ctx.result_to_rank);
  }

  switch (ctx.curve_type) {
    case CURVE_TYPE_CURVE25519: {
      config.set_curve_type(::psi::CurveType::CURVE_25519);
      YACL_ENFORCE(ctx.hash_type == HASH_TYPE_SHA_256,
                   "Currently only support sha256 hash for curve25519");
      YACL_ENFORCE(
          ctx.hash_to_curve_strategy ==
              HASH_TO_CURVE_STRATEGY_DIRECT_HASH_AS_POINT_X,
          "Currently only support DIRECT_HASH_AS_POINT_X for curve25519");
      YACL_ENFORCE(ctx.point_octet_format == POINT_OCTET_FORMAT_UNCOMPRESSED,
                   "Currently only support uncompressed format for curve25519");
      break;
    }
    case CURVE_TYPE_SM2: {
      config.set_curve_type(::psi::CurveType::CURVE_SM2);
      YACL_ENFORCE(ctx.hash_type == HASH_TYPE_SHA_256,
                   "Currently only support sha256 hash for sm2");
      YACL_ENFORCE(
          ctx.hash_to_curve_strategy == HASH_TO_CURVE_STRATEGY_TRY_AND_REHASH,
          "Currently only support TRY_AND_REHASH for sm2");
      YACL_ENFORCE(
          ctx.point_octet_format == POINT_OCTET_FORMAT_X962_COMPRESSED,
          "Currently only support ANSI X9.62 compressed format for sm2");
      break;
    }
    default:
      YACL_THROW("Unspecified curve type: {}", ctx.curve_type);
  }

  return std::make_unique<::psi::BucketPsi>(config, ctx.ic_ctx->lctx, true);
}

std::unique_ptr<::psi::CsvChecker> CheckInput(const EcdhPsiContext &ctx) {
  auto field_list = absl::StrSplit(GetPsiInputFileFieldNames(), ',');
  std::vector<std::string> selected_fields(field_list.begin(),
                                           field_list.end());
  return ::psi::CheckInput(ctx.ic_ctx->lctx, GetPsiInputFileName(),
                           selected_fields, false, true);
}

}  // namespace ic_impl::algo::psi::v2
