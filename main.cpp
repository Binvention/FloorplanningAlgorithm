/***********************************************************************************
 * Program: Simulated Annealing algorithm
 * @brief This program executes a simulated annealing algorithm for balancing floor
 * planning 
 * Author: Brandon Baird
************************************************************************************/

#include <iostream> 
#include <fstream> 
#include <string>
#include <sstream>
#include <list>
#include "SNode.h"

//Initial NPEs for annealing
const std::string initialVerticalNPE = "12V3V4V5V6V7V8V9VaVbVcVdVeVfVgViVjVkVlV";
const std::string initialHorizontalNPE = "12H3H4H5H6H7H8H9HaHbHcHdHeHfHgHiHjHkHlH";
const std::string initialOtherNPE = "213546H7VHVa8V9HcVHgHibdHkVHfeHVlHVjHVH";

//functions
bool isValidNPE(std::string npe);
void getCells(std::string filename, std::list<SNode> &cells);
float cost(std::string npe ,std::list<SNode> &cells);
SNode * generateTree(std::string npe, std::list<SNode> &cells, std::list<SNode> &operators);

/***********************************************************************************
 * Function: main
 * @brief the main routine of the program
************************************************************************************/
int main (int argc , const char* argv[])
{
   //Cells of the floorplan
   std::list<SNode> cells;
   if (argc > 1)
   {
      getCells(std::string(argv[1]),cells);
   }
   else
   {
      getCells("input_file.txt",cells);
   }
   std::cout << "NPE: " << initialVerticalNPE << "\n";
   std::cout << "Cost: " << cost(initialVerticalNPE,cells) << "\n";
   std::cout << "NPE: " << initialHorizontalNPE << "\n";
   std::cout << "Cost: " << cost(initialHorizontalNPE,cells) << "\n";
   std::cout << "NPE: " << initialOtherNPE << "\n";
   std::cout << "Cost: " << cost(initialOtherNPE,cells) << std::endl;

   return 0;
}

/***********************************************************************************
 * Function: isValidNPE
 * @brief verifies the the provided Normalized Polish Expression is valid
 * @param npe the Normalized Polish Expression as a string
 * @return true if valid false otherwise
************************************************************************************/
bool isValidNPE(std::string npe)
{
   int operands = 0;
   int operators = 0;
   for (int i = 0; i < npe.size(); i++) 
   {
      //if it is an operator check for repeats add to the operator count
      if ((npe[i] == 'V')||(npe[i] == 'H'))
      {
         //make sure there are no repeat operators 
         if(i+1 < npe.size())
         {
            if (npe[i] == npe [i+1])
            {
               return false;
            }
         }
         operators++;
      }
      else //if it is an operand make sure it is unique and add to operand count
      {
         //if another instance is found return false
         if (npe.find(npe[i],i+1)!=std::string::npos)
         {
            return false;
         }
         operands++;
      }
      //make sure it meets balloting property
      if(operands <= operators)
      {
         return false;
      }
   }
   if (operators == operands -1)
   {
      return true;
   }
   return false;
}

/***********************************************************************************
 * Function: getCells
 * @brief loads the cells for the floorplan from the designated file
 * @param filename the name of the file containing the cells
************************************************************************************/
void getCells(std::string filename, std::list<SNode> &cells)
{
   if (filename == "")
   {
      std::cout << "Please enter file name: ";
      std::cin.clear();
      std::cin.ignore();
      getline(std::cin,filename);
   }
    // open the file
   std::ifstream fin(filename);
   if (fin.fail())
   {
      throw "Unable to open file";
   }
   
   //extract the cells from the file
   std::string line;
   while(getline(fin,line))
   {
      std::stringstream stream(line);
      char name;
      float area;
      float aspectRatio;
      stream >> name;
      stream >> area;
      stream >> aspectRatio;
      cells.push_back(SNode(name, area, aspectRatio));
   }
}

/***********************************************************************************
 * Function: cost
 * @brief calculates the cost of the Normalized Polish expression given the cells
 *    provided
 * @param npe the Normalized Polish expression
 * @param cells the cells to be arranged
 * @return the area of the overall floorplan
************************************************************************************/
float cost(std::string npe ,std::list<SNode> &cells)
{
   //create tree from npe
   std::list<SNode> operators; //list to store operators
   SNode * root = generateTree(npe, cells, operators);
   return root->calcMinArea();
}

/***********************************************************************************
 * Function: generateTree
 * @brief generates a slicing tree from a Normalized Polar Expression 
 * @param npe the Normalized Polar Expression
 * @param cells the cells to be organized
 * @param operators this should be empty but is used to store the operators of the 
 *    tree
 * @return returns a pointer to the root of the tree which is also the first 
 *    element in the operators list
************************************************************************************/
SNode * generateTree(std::string npe, std::list<SNode> &cells, std::list<SNode> &operators)
{
   operators.clear();
   //Validate npe
   if(!isValidNPE(npe))
   {
      std::cout << "Invalid NPE!";
      throw "Invalid NPE!";
   }
   //generate tree
   std::string::reverse_iterator currentChar = npe.rbegin(); //start from back of string
   operators.push_back(SNode(*currentChar)); //since it is npe we know this will be an operator
   SNode * current = &operators.back(); //set root 
   currentChar++;
   while (currentChar != npe.rend()) //while there are still characters in NPE
   {
      if((*currentChar == 'V') || (*currentChar == 'H')) //its an operator
      {
         operators.push_back(SNode(*currentChar));
         if(current->right) //assign right when possible left if not
         {
            current->left = &operators.back();
            current->left->parent = current;
         }
         else
         {
            current->right = &operators.back();
            current->right->parent = current;
         }
         current = &operators.back();
      }
      else //its a operand
      {
         //find the opperand in the cells
         SNode * child = NULL;
         for (std::list<SNode>::iterator i = cells.begin(); i != cells.end(); i++)
         {
            if (i->name == *currentChar)
            {
               child = &(*i);
            }
         }
         //assign it to right if possible left otherwise
         if(child)
         {
            if(current->right) 
            {
               current->left = child;
               while ((current != &operators.front()) && (current->left))
               {
                  current = current->parent;
               }
            }
            else
            {
               current->right = child;
            }
         }
         else //item not found in cells 
         {
            throw "Cell data not valid!";
         }
      }
      currentChar++;
   }
   return &operators.front();
}