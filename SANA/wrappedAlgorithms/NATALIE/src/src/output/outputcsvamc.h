/*
 * outputcsvamc.h
 *
 *  Created on: 21-mar-2012
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTCSVAMC_H_
#define OUTPUTCSVAMC_H_

#include <ostream>
#include <fstream>
#include <set>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputCsvAmc : public Output<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef Output<Graph, BpGraph> Parent;

  typedef enum {
                 CSV_MATCHED,
                 CSV_UNMATCHED_IN_G1,
                 CSV_UNMATCHED_IN_G2,
                 CSV_ALIGNMENT,
               } OutputFormatType;

  using Parent::_matchingGraph;

private:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  typedef typename BpGraph::Node BpNode;
  typedef typename BpGraph::Edge BpEdge;
  typedef typename BpGraph::NodeIt BpNodeIt;
  typedef typename BpGraph::EdgeIt BpEdgeIt;
  typedef typename BpGraph::IncEdgeIt BpIncEdgeIt;
  typedef typename BpGraph::RedNode BpRedNode;
  typedef typename BpGraph::BlueNode BpBlueNode;
  typedef typename BpGraph::RedNodeIt BpRedNodeIt;
  typedef typename BpGraph::BlueNodeIt BpBlueNodeIt;
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  typedef typename Parent::BpMatchingMapType BpMatchingMapType;
  typedef typename Parent::OutputType OutputType;
  typedef typename std::set<BpBlueNode> BpBlueNodeSet;
  typedef typename BpBlueNodeSet::const_iterator BpBlueNodeSetIt;

  struct Entry {
    Node _node1;
    Node _node2;
    BpEdge _bpEdge;

    Entry(const Node node1,
          const Node node2,
          const BpEdge bpEdge)
      : _node1(node1)
      , _node2(node2)
      , _bpEdge(bpEdge)
    {
    }
  };

  typedef typename std::vector<Entry> EntryVector;
  typedef typename EntryVector::const_iterator EntryVectorIt;

  OutputFormatType _outputKind;

public:
  OutputCsvAmc(const MatchingGraphType& matchingGraph,
            OutputFormatType outputKind)
    : Parent(matchingGraph)
    , _outputKind(outputKind)
  {
  }

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    switch (_outputKind)
    {
      case CSV_MATCHED:
        return "-matched.csv";
      case CSV_UNMATCHED_IN_G1:
        return "-unmatched1.csv";
      case CSV_UNMATCHED_IN_G2:
        return "-unmatched2.csv";
      case CSV_ALIGNMENT:
        return "-alignment.csv";
      default:
        return ".csv";
    }
  }
};

template<typename GR, typename BGR>
inline void OutputCsvAmc<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType,
                                      std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();

  // we assume that G_1 is the query network and G_2 the target network
  EntryVector entries;
  BpBlueNodeSet blueNodeSet;

  // generate the query nodes first that have a match
  for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
  {
    BpEdge e = matchingMap[r];
    if (e != lemon::INVALID)
    {
      BpBlueNode b = gm.blueNode(e);
      blueNodeSet.insert(b);

      Entry entry(_matchingGraph.mapGmToG1(r), _matchingGraph.mapGmToG2(b), e);
      entries.push_back(entry);
    }
  }

  // time to generate output, we start with nodes
  switch (_outputKind)
  {
    case CSV_MATCHED:
      out << "\"Mouse gene\"" << ","
          << "\"Mouse gene\"" << ","
          << "\"Mouse co-expression\"" << ","
          << "\"Human gene\"" << ","
          << "\"Human gene\"" << ","
          << "\"Human co-expression\"" << std::endl;
      break;
    case CSV_UNMATCHED_IN_G1:
      out << "\"Human gene\"" << ","
          << "\"Human gene\"" << ","
          << "\"Human co-expression\"" << ","
          << "\"Mouse gene\"" << ","
          << "\"Mouse gene\""
          << std::endl;
      break;
    case CSV_UNMATCHED_IN_G2:
      out << "\"Mouse gene\"" << ","
          << "\"Mouse gene\"" << ","
          << "\"Mouse co-expression\"" << ","
          << "\"Human gene\"" << ","
          << "\"Human gene\""
          << std::endl;
      break;
    case CSV_ALIGNMENT:
      out << "\"Mouse gene\"" << ","
          << "\"Human gene\"" << ","
          << "\"Bit score\"" << std::endl;
      break;
    default:
      break;
  }

  int nodeIdx1 = 0;
  for (EntryVectorIt it1 = entries.begin(); it1 != entries.end(); it1++, nodeIdx1++)
  {
    if (_outputKind == CSV_ALIGNMENT &&
      it1->_node1 != lemon::INVALID && it1->_node2 != lemon::INVALID)
    {
      out << "\"" << _matchingGraph.getLabelG1(it1->_node1) << "\"" << ","
          << "\"" << _matchingGraph.getLabelG2(it1->_node2) << "\"" << ","
          << _matchingGraph.getWeightGm(it1->_bpEdge) << std::endl;
      continue;
    }

    int nodeIdx2 = nodeIdx1;
    for (EntryVectorIt it2 = it1; it2 != entries.end(); it2++, nodeIdx2++)
    {
      bool queryEdge = (it1->_node1 != lemon::INVALID && it2->_node1 != lemon::INVALID) ?
        _matchingGraph.getEdgeG1(it1->_node1, it2->_node1) != lemon::INVALID : false;

      bool targetEdge = (it1->_node2 != lemon::INVALID && it2->_node2 != lemon::INVALID) ?
        _matchingGraph.getEdgeG2(it1->_node2, it2->_node2) != lemon::INVALID : false;

      switch (_outputKind)
      {
        case CSV_MATCHED:
          if (queryEdge && targetEdge)
            out << "\"" << _matchingGraph.getLabelG1(it1->_node1) << "\"" << ","
                << "\"" << _matchingGraph.getLabelG1(it2->_node1) << "\"" << ","
                << "\"" << _matchingGraph.getWeightG1(_matchingGraph.getEdgeG1(it1->_node1, it2->_node1)) << "\"" << ","
                << "\"" << _matchingGraph.getLabelG2(it1->_node2) << "\"" << ","
                << "\"" << _matchingGraph.getLabelG2(it2->_node2) << "\"" << ","
                << "\"" << _matchingGraph.getWeightG2(_matchingGraph.getEdgeG2(it1->_node2, it2->_node2))
                << std::endl;
          break;
        case CSV_UNMATCHED_IN_G1:
          //if (targetEdge && it1->_node1 != lemon::INVALID && it2->_node1 != lemon::INVALID)
          if (targetEdge && !queryEdge)
            out << "\"" << _matchingGraph.getLabelG2(it1->_node2) << "\"" << ","
                << "\"" << _matchingGraph.getLabelG2(it2->_node2) << "\"" << ","
                << "\"" << _matchingGraph.getWeightG2(_matchingGraph.getEdgeG2(it1->_node2, it2->_node2)) << "\","
                << "\"" << _matchingGraph.getLabelG1(it1->_node1) << "\"" << ","
                << "\"" << _matchingGraph.getLabelG1(it2->_node1) << "\""
                << std::endl;
          break;
        case CSV_UNMATCHED_IN_G2:
          //if (queryEdge && it1->_node2 != lemon::INVALID && it2->_node2 != lemon::INVALID)
          if (queryEdge && !targetEdge)
            out << "\"" << _matchingGraph.getLabelG1(it1->_node1) << "\"" << ","
                << "\"" << _matchingGraph.getLabelG1(it2->_node1) << "\"" << ","
                << "\"" << _matchingGraph.getWeightG1(_matchingGraph.getEdgeG1(it1->_node1, it2->_node1)) << "\","
                << "\"" << (it1->_node2 != lemon::INVALID ? _matchingGraph.getLabelG2(it1->_node2) : "") << "\"" << ","
                << "\"" << (it2->_node2 != lemon::INVALID ? _matchingGraph.getLabelG2(it2->_node2) : "") << "\""
                << std::endl;
          break;
        default:
          continue;
      }
    }
  }
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTCSVAMC_H_ */
