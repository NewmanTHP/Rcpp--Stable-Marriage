#include <map>
#include <list>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm> 
#include <iterator>

#include <Rcpp.h>
using namespace Rcpp;

//[[Rcpp::plugins(cpp17)]]

List vector_of_pairs_to_vector_of_vector(std::vector<std::pair<std::string, std::string> > input){
  std::vector<std::vector<std::string> > output;
  for(auto&a: input){
    output.push_back(std::vector<std::string> {a.first, a.second});
  }
  return wrap(output);
}

std::map<std::string,std::vector<std::string> > dataframe_to_map(DataFrame ex) {
  // Extracting the keys from the dataframe
  CharacterVector keys = ex.names();
  
  std::map<std::string,std::vector<std::string> > output;
  // need to populate the output with keys and values from dataframe ex
  for (int i=0; i<keys.size(); i++) {
    
    std::string key = as<std::string>(keys[i]);
    
    std::vector<std::string> values = ex[key];
    
    output[key] = values;
  }
  return output;
}

struct RetrieveKey
{
    template <typename T>
    typename T::first_type operator()(T keyValuePair) const
    {
        return keyValuePair.first;
    }
};

struct CompareFirst
{
  CompareFirst(std::string val) : val_(val) {}
  bool operator()(const std::pair<std::string,std::string>& elem) const {
    return val_ == elem.second;
  }
  private:
    std::string val_;
};

//[[Rcpp::export]]
List finding_stable_matching(DataFrame r_ex_male, DataFrame r_ex_female)
{
  std::map<std::string,std::vector<std::string> > ex_male;
  std::map<std::string,std::vector<std::string> > ex_female;
  ex_male = dataframe_to_map(r_ex_male);
  ex_female = dataframe_to_map(r_ex_female);
  
    std::vector<std::string> list_male;

    // Retrieve all keys from male map
    std::transform(ex_male.begin(), ex_male.end(), back_inserter(list_male), RetrieveKey());
    // Initialising k to 0 and n to the number of males
    int k {0} ;
    int n = list_male.size();
    // Creating a vector of original matchings (i.e. all females matched to "undesirable male")
    std::string omega{"Om"};
    std::vector<std::pair<std::string, std::string> > matching;
    matching = {{omega,"a"},{omega,"b"},{omega,"c"},{omega,"d"}};

    while (k<n){
        // Selecting the kth male from the list (i.e. the current suitor)
        std::string X{list_male[k]};
        while (X!=omega){
            // Preferred choice on current male's list
            std::string x{ex_male[X][0]};
            // Obtaining the couple of current male's preferred choice
            std::vector<std::pair<std::string,std::string> >::iterator if_x_couple = std::find_if(matching.begin(),matching.end(), CompareFirst(x));
            int current_pair_position = std::distance(matching.begin(), if_x_couple);
            std::pair<std::string,std::string> x_couple{matching[current_pair_position]};
            // Male with whom current male's preferred choice is currently coupled
            std::string x_fiance{x_couple.first};
            // Obtain a list of preference for current male's preferred choice
            std::vector<std::string> list_of_x_choice = ex_female[x];
            // Current ranking of male suitor in the preference of his preferred choice
            std::vector<std::string>::iterator if_X = std::find(list_of_x_choice.begin(), list_of_x_choice.end(), X);
            int current_male_position_in_x = std::distance(list_of_x_choice.begin(), if_X);
            // Current ranking of the partner of the current suitor's preferred choice
            std::vector<std::string>::iterator if_x_fiance = std::find(list_of_x_choice.begin(), list_of_x_choice.end(), x_fiance);
            int current_partner_position_in_x = std::distance(list_of_x_choice.begin(), if_x_fiance);
            if (current_male_position_in_x<current_partner_position_in_x){
                // Matching x (i.e. the female) with the current suitor
                x_couple.first = X;
                matching[current_pair_position] = x_couple;
                // Setting the new suitor to be the old partner of x
                X = x_fiance;
            }
            if (X!=omega){
                    // removing x (i.e. the female) from the list of preference of X (i.e. the new suitor)
                    remove(ex_male[X].begin(),ex_male[X].end(),x);
            }

        }
        // Increase K by 1
        k++;
    }
    return vector_of_pairs_to_vector_of_vector(matching);
}

// int main()
// {
//     	std::map<std::string,std::vector<std::string>> ex_male;
//    	std::map<std::string,std::vector<std::string>> ex_female;
//         
// 	ex_male["A"] = {"c","b","d","a"};
// 	ex_male["B"] = {"b","a","c","d"};
// 	ex_male["C"] = {"b","d","a","c"};
// 	ex_male["D"] = {"c","a","d","b"};
// 	
// 	ex_female["a"] = {"A","B","D","C","Om"};
// 	ex_female["b"] = {"C","A","D","B","Om"};
// 	ex_female["c"] = {"C","B","D","A", "Om"};
// 	ex_female["d"] = {"B","A","C","D", "Om"};
// 
// 	std::vector<std::pair<std::string, std::string>> matches;
// 	matches = finding_stable_matching(ex_male, ex_female);
// 	for (const auto& p : matches)
// 	  {
// 	    std::cout << "("<< std::get<0>(p) << "," << std::get<1>(p) << ")"<< std::endl;
// 	  }
//     return 0;
// }
