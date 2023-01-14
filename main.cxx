#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <tuple>
#include <cmath>

std::string transform(const std::string& path) {
  std::ifstream in(path);
  std::stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

struct Line {
  unsigned year;
  unsigned month;
  unsigned day;
  unsigned hour;
  unsigned minute;
  float second;
};

Line line(const std::string& line) {
  Line ret = {};
  const auto comma = line.find(",");
  if (comma == std::string::npos) {
    throw std::runtime_error("no comma in the line " + line);
  }
  const std::string astr = line.substr(0, comma);
  const std::string bstr = line.substr(comma + 1);
  ret.year = std::stoul(astr.substr(0, 4));
  ret.month = std::stoul(astr.substr(5, 2));
  ret.day = std::stoul(astr.substr(8, 2));
  ret.hour = std::stoul(bstr.substr(0, 2));
  ret.minute = std::stoul(bstr.substr(3, 2));
  ret.second = std::stof(bstr.substr(6, 5));
  return ret;
}

std::vector<Line>
from(const std::string& log) {
  auto ret = std::vector<Line>();
  std::stringstream ss(log);
  {
    std::string d;
    while (ss >> d) {
      if (d.find("2020-11-") != std::string::npos) {
        std::string t;
        ss >> t;
        ret.push_back(line(d + "," + t));
      }
    }
  }
  return ret;
}

std::tuple<double, double> getMeanAndStddev(
    const typename std::vector<std::size_t>::const_iterator& start,
    const typename std::vector<std::size_t>::const_iterator& finish
) {
  auto sum = 0.;
  for (auto i = start; i != finish; i++) {
    sum += *i;
  }
  const auto size = std::distance(start, finish);
  const auto mean = static_cast<double>(sum) / size;
  auto stddev = 0.;
  for (auto i = start; i != finish; i++) {
    stddev += (*i - mean) * (*i - mean);
  }
  stddev *= 1. / (size - 1);
  stddev = std::sqrt(stddev);
  return {mean, stddev};
};

std::vector<Line> byDay(const std::vector<Line>& data, unsigned day) {
  auto ret = std::vector<Line>();
  for (const auto& line : data) {
    if (line.day == day) {
      ret.push_back(line);
    }
  }
  return ret;
}

std::vector<Line> byHours(
  const std::vector<Line>& data,
  unsigned min,
  unsigned max
) {
  auto ret = std::vector<Line>();
  for (const auto& line : data) {
    if (line.hour >= min && line.hour <= max) {
      ret.push_back(line);
    }
  }
  return ret;
}

int main() {
  const std::string db = "ddd.csv";
  const auto data = from(transform(db));
  constexpr unsigned DAYS = 30;
  std::array<std::vector<Line>, DAYS> day;
  for (auto i = 1u; i <= DAYS; i++) {
    day[i - 1] = byHours(byDay(data, i), 8, 12);
  }
  std::vector<std::size_t> requests;
  for (auto i = 1u; i <= DAYS; i++) {
    requests.push_back(day[i - 1].size());
  }
  for (auto i = 0u; i < requests.size(); i++) {
    std::cout << "day " << i << ": " << requests[i] << std::endl;
  }
  const auto [mean, stddev] = getMeanAndStddev(
      requests.begin(),
      requests.end()
  );
  std::cout << "mean: " << mean << "\tstddev: " << stddev << std::endl;

  return 0;
}
