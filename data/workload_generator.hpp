#ifndef WORKLOAD_GENERATOR_H
#define WORKLOAD_GENERATOR_H

#include <string>
#include <vector>

// Define a structure for query workload
struct Query {
    enum class Type { EmptyPoint, NonEmptyPoint, Range, Write };
    Type type;
    // Add other fields specific to each query type
};

std::vector<Query> GenerateWorkload(
    double emptyPointPercentage,
    double nonEmptyPointPercentage,
    double rangeQueryPercentage,
    double writeQueryPercentage,
    int totalQueries
);

void MeasurePerformance(const std::vector<Query>& workload);

#endif