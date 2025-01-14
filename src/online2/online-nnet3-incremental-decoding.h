// online2/online-nnet3-incremental-decoding.h

// Copyright      2019  Zhehuai Chen

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.


#ifndef KALDI_ONLINE2_ONLINE_NNET3_INCREMENTAL_DECODING_H_
#define KALDI_ONLINE2_ONLINE_NNET3_INCREMENTAL_DECODING_H_

#include <string>
#include <vector>
#include <deque>

#include "nnet3/decodable-online-looped.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "base/kaldi-error.h"
#include "itf/online-feature-itf.h"
#include "online2/online-endpoint.h"
#include "online2/online-nnet2-feature-pipeline.h"
#include "decoder/lattice-incremental-online-decoder.h"
#include "hmm/transition-model.h"
#include "hmm/posterior.h"

namespace kaldi {
/// @addtogroup  onlinedecoding OnlineDecoding
/// @{


/**
   You will instantiate this class when you want to decode a single utterance
   using the online-decoding setup for neural nets.  The template will be
   instantiated only for FST = fst::Fst<fst::StdArc> and FST = fst::GrammarFst.
*/

template <typename FST>
class SingleUtteranceNnet3IncrementalDecoderTpl {
 public:

  // Constructor. The pointer 'features' is not being given to this class to own
  // and deallocate, it is owned externally.
  SingleUtteranceNnet3IncrementalDecoderTpl(const LatticeIncrementalDecoderConfig &decoder_opts,
                                            const TransitionModel &trans_model,
                                            const nnet3::DecodableNnetSimpleLoopedInfo &info,
                                            const FST &fst,
                                            OnlineNnet2FeaturePipeline *features);

  /// Initializes the decoding and sets the frame offset of the underlying
  /// decodable object. This method is called by the constructor. You can also
  /// call this method when you want to reset the decoder state, but want to
  /// keep using the same decodable object, e.g. in case of an endpoint.
  void InitDecoding(int32 frame_offset = 0);

  /// Advances the decoding as far as we can.
  void AdvanceDecoding();

  /// Finalizes the decoding. Cleans up and prunes remaining tokens, so the
  /// GetLattice() call will return faster.  You must not call this before
  /// calling (TerminateDecoding() or InputIsFinished()) and then Wait().
  void FinalizeDecoding() { decoder_.FinalizeDecoding(); }

  int32 NumFramesDecoded() const { return decoder_.NumFramesDecoded(); }

  int32 NumFramesInLattice() const { return decoder_.NumFramesInLattice(); }

  /* Gets the lattice.  The output lattice has any acoustic scaling in it
     (which will typically be desirable in an online-decoding context); if you
     want an un-scaled lattice, scale it using ScaleLattice() with the inverse
     of the acoustic weight.

         @param [in] num_frames_to_include  The number of frames you want
                  to be included in the lattice.  Must be in the range
                  [NumFramesInLattice().. NumFramesDecoded()].  If you
                  make it a few frames less than NumFramesDecoded(), it
                  will save significant computation.
         @param [in] use_final_probs   True if you want the lattice to
                  contain final-probs (if at least one state was final
                  on the most recently decoded frame).  Must be false
                  if num_frames_to_include < NumFramesDecoded().
                  Must be true if you have previously called
                  FinalizeDecoding().
  */
  const CompactLattice &GetLattice(int32 num_frames_to_include,
                                      bool use_final_probs = false) {
    return decoder_.GetLattice(num_frames_to_include, use_final_probs);
  }





  /// Outputs an FST corresponding to the single best path through the current
  /// lattice. If "use_final_probs" is true AND we reached the final-state of
  /// the graph then it will include those as final-probs, else it will treat
  /// all final-probs as one.
  void GetBestPath(bool end_of_utterance,
                   Lattice *best_path) const;


  /// This function calls EndpointDetected from online-endpoint.h,
  /// with the required arguments.
  int EndpointDetected(const OnlineEndpointConfig &config);

  const LatticeIncrementalOnlineDecoderTpl<FST> &Decoder() const { return decoder_; }

  ~SingleUtteranceNnet3IncrementalDecoderTpl() { }
 private:

  const LatticeIncrementalDecoderConfig &decoder_opts_;

  // this is remembered from the constructor; it's ultimately
  // derived from calling FrameShiftInSeconds() on the feature pipeline.
  BaseFloat input_feature_frame_shift_in_seconds_;

  // we need to keep a reference to the transition model around only because
  // it's needed by the endpointing code.
  const TransitionModel &trans_model_;

  nnet3::DecodableAmNnetLoopedOnline decodable_;

  LatticeIncrementalOnlineDecoderTpl<FST> decoder_;

};


typedef SingleUtteranceNnet3IncrementalDecoderTpl<fst::Fst<fst::StdArc> > SingleUtteranceNnet3IncrementalDecoder;

/// @} End of "addtogroup onlinedecoding"

}  // namespace kaldi



#endif  // KALDI_ONLINE2_ONLINE_NNET3_DECODING_H_
