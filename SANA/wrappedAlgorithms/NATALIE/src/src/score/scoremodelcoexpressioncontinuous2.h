/*
 * scoremodelcoexpressioncontinuous2.h
 *
 *  Created on: 17-jan-2012
 *     Authors: M. El-Kebir and M.E. van der Wees
 */

#ifndef SCOREMODELCOEXPRESSIONCONTINUOUS2_H_
#define SCOREMODELCOEXPRESSIONCONTINUOUS2_H_

#include <assert.h>
#include <string>
#include <lemon/core.h>
#include "score/scoremodel.h"
#include "input/matchinggraph.h"
#include "verbose.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class ScoreModelCoexpressionContinuous2 : public ScoreModel<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef ScoreModel<GR, BGR> Parent;

protected:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  typedef typename Parent::BpNode BpNode;
  typedef typename Parent::BpEdge BpEdge;
  typedef typename Parent::BpNodeIt BpNodeIt;
  typedef typename Parent::BpEdgeIt BpEdgeIt;
  typedef typename Parent::BpIncEdgeIt BpIncEdgeIt;
  typedef typename Parent::BpRedNode BpRedNode;
  typedef typename Parent::BpBlueNode BpBlueNode;
  typedef typename Parent::BpRedNodeIt BpRedNodeIt;
  typedef typename Parent::BpBlueNodeIt BpBlueNodeIt;

  /// Weights on original edges
  typedef typename Parent::WeightEdgeMap WeightEdgeMap;
  /// Weights on matching edges
  typedef typename Parent::BpWeightEdgeMap BpWeightEdgeMap;

  /// Matching graph
  using Parent::_matchingGraph;
  /// Scaling
  using Parent::_beta;
  /// Max score used for normalisation
  using Parent::_maxScore;
  /// Min score used for normalisation
  using Parent::_minScore;

  double _corrThreshold;

public:
  /// Constructor
  ScoreModelCoexpressionContinuous2(const MatchingGraph<Graph, BpGraph>& matchingGraph, double beta, double corrThreshold);
  /// Destructor
  virtual ~ScoreModelCoexpressionContinuous2() {}

  std::string getScoreFunction() const { return "Continuous 2"; }

  /// Return weight of edge in G_m
  double getWeightGm(BpEdge ik) const
  {
    // Min-max normalisation
    if (_minScore == _maxScore)
      return 0;
    else
      return (1 - _beta) * (_matchingGraph.getWeightGm(ik) - _minScore) / (_maxScore - _minScore);
  }

  /// Return weight of pair of edges (ij, kl) in G_1 and G_2 resp
  double getWeightG1G2(Edge ij, Edge kl) const
  {
    double weight1 = _matchingGraph.getWeightG1(ij);
    double weight2 = _matchingGraph.getWeightG2(kl);

    if (fabs(weight1) < _corrThreshold || fabs(weight2) < _corrThreshold)
      return 0;

    // Penalty for opposite signs in coexpression values
    else if ((weight1 <= -_corrThreshold && weight2 >= _corrThreshold) || (weight2 <= -_corrThreshold && weight1 >= _corrThreshold))
        return -_beta;

    // Profit for equal signs in coexpression values
    else
      return _beta * 0.5 * (fabs(weight1) + fabs(weight2)) * (((1 - _corrThreshold) - fabs(weight1 - weight2)) / (1 - _corrThreshold));
  }
};

template<typename GR, typename BGR>
inline ScoreModelCoexpressionContinuous2<GR, BGR>::ScoreModelCoexpressionContinuous2(const MatchingGraph<Graph, BpGraph>& matchingGraph, double beta, double corrThreshold)
  : Parent(matchingGraph, beta, false, true)
  , _corrThreshold(corrThreshold)
{
}

} // namespace gna
} // namespace nina

#endif /* SCOREMODELCOEXPRESSIONCONTINUOUS2_H_ */
