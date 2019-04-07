/** Copyright 2018 RWTH Aachen University. All rights reserved.
 *
 *  Licensed under the RWTH ASR License (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.hltpr.rwth-aachen.de/rwth-asr/rwth-asr-license.html
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "SequenceFilter.hh"

using namespace Flow;

SequenceFilterNode::SequenceFilterNode(const Core::Configuration& c)
        : Core::Component(c),
          Node(c),
          featureIndex_(0) {
    addInputs(2);
    addOutputs(1);
}

void SequenceFilterNode::updateSelection(const Timestamp& timestamp) {
    while (!selection_ || !selection_->contains(timestamp)) {
        featureIndex_ = 0;
        if (!getData(1, selection_)) {
            criticalError("In data stream, no object contained the interval [%f..%f].",
                          timestamp.startTime(), timestamp.endTime());
        }
    }
}

bool SequenceFilterNode::configure() {
    selection_.reset();

    Core::Ref<Attributes> attributesSelection(new Attributes);
    getInputAttributes(1, *attributesSelection);
    if (!configureDatatype(attributesSelection, Vector<bool>::type()))
        return false;

    return putOutputAttributes(0, getInputAttributes(0));
}

bool SequenceFilterNode::work(PortId p) {
    DataPtr<Timestamp> in;
    while (getData(0, in)) {
        updateSelection(*in);
        Vector<bool>& selection = *selection_;
        if (featureIndex_ >= selection.size())
            criticalError("Input stream (%d) is longer than selection (%zd).", featureIndex_ + 1, selection.size());
        if (selection[featureIndex_++])
            return putData(0, in.get());
    }
    return putData(0, in.get());
}
