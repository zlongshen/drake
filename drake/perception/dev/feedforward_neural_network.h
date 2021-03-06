#pragma once

#include <memory>
#include "drake/common/drake_copyable.h"
#include "drake/common/eigen_types.h"
#include "drake/perception/dev/neural_network.h"
#include "drake/systems/framework/leaf_system.h"

#include <Eigen/Dense>

namespace drake {
namespace perception {

enum class LayerType { FullyConnected, Convolutional, Dropout };
enum class NonlinearityType { Relu, Sigmoid, Atan };

template <typename T>
class FeedforwardNeuralNetwork : public NeuralNetwork<T> {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(FeedforwardNeuralNetwork)

  /// TODO(nikos-tri): Add documentation
  /// The number of layers does not count the input layer
  FeedforwardNeuralNetwork(const std::vector<MatrixX<T>>& W,
                           const std::vector<VectorX<T>>& b,
                           const std::vector<LayerType>& layers,
                           const std::vector<NonlinearityType>& nonlinearities);

  std::unique_ptr<FeedforwardNeuralNetwork<AutoDiffXd>> ToAutoDiffXd() const {
    return std::unique_ptr<FeedforwardNeuralNetwork<AutoDiffXd>>(
        DoToAutoDiffXd());
  }

  const systems::InputPortDescriptor<T>& input() const;
  const systems::OutputPort<T>& output() const;

  int get_num_layers() const;
  int get_num_inputs() const;
  int get_num_outputs() const;
  std::unique_ptr<MatrixX<T>> get_weight_matrix(
      int index, const systems::Context<T>& context) const;
  std::unique_ptr<VectorX<T>> get_bias_vector(
      int index, const systems::Context<T>& context) const;
  std::unique_ptr<systems::BasicVector<T>> EncodeWeightsToBasicVector(
      const MatrixX<T>& matrix) const;
  std::unique_ptr<MatrixX<T>> DecodeWeightsFromBasicVector(
      int rows, int cols, const systems::BasicVector<T>& vector) const;

 private:
  void DoCalcOutput(const systems::Context<T>& context,
                    systems::BasicVector<T>* output) const;
  VectorX<T> EvaluateLayer(const VectorX<T>& layerInput, MatrixX<T> Weights,
                           VectorX<T> bias, LayerType layer,
                           NonlinearityType nonlinearity) const;
  VectorX<T> relu(VectorX<T> in) const;

  FeedforwardNeuralNetwork<AutoDiffXd>* DoToAutoDiffXd() const override;

  // TODO(nikos-tri)
  // FeedforwardNeuralNetwork<symbolic::Expression>* DoToSymbolic() const
  // override;

  // To improve readability of the main computation
  const VectorX<T> ReadInput(const systems::Context<T>& context) const;
  void WriteOutput(const VectorX<T> value,
                   systems::BasicVector<T>* output) const;

  // The types of layers and nonlinearities in this NN
  std::vector<LayerType> layers_;
  std::vector<NonlinearityType> nonlinearities_;

  // Copies of weights and biases, as passed to the constructor
  // Need to store these because they are needed by DoToAutoDiffXd
  std::vector<MatrixX<T>> weights_matrices_;
  std::vector<VectorX<T>> bias_vectors_;
  // Need these for coding and decoding into BasicVector
  std::vector<int> rows_;
  std::vector<int> cols_;

  // Indices for weights and biases, which are stored as params in the Context
  std::vector<int> weight_indices_;
  std::vector<int> bias_indices_;

  // Indices for inputs and outputs
  const int input_index_;
  const int output_index_;

  // Structural parameters inferred from weights and biases given to constructor
  int num_inputs_;
  int num_outputs_;
  int num_layers_;
};

}  // namespace automotive
}  // namespace drake
