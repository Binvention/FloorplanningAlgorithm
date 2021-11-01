/***********************************************************************************
 * File: SNode.h
 * @brief Contains the SNode class for performing floorplanning using slicing trees
 * Author: Brandon Baird
************************************************************************************/

#ifndef SNODE_H
#define SNODE_H

#include <math.h>
#include <list>

/***********************************************************************************
 * Struct: Dimensions
 * @brief Contains the dimensions of the cell (height and width)
************************************************************************************/
struct Dimensions
{
   float height;
   float width;
   std::list<Dimensions>::iterator rSelected;
   std::list<Dimensions>::iterator lSelected;
};

bool operator== (const Dimensions &lhs, const Dimensions &rhs);

/***********************************************************************************
 * Class: SNode
 * @brief provides functionality for a slicing tree node
************************************************************************************/
class SNode 
{
public:
   bool isOperator;
   bool fixed;
   char name;
   float aspectRatio;
   float area;
   std::list<Dimensions> sizes;
   Dimensions selected;
   SNode * right;
   SNode * left;
   SNode * parent;
   SNode(char name, float area, float aspectRatio);
   SNode(char name, float area, float aspectRatio, bool fixed);
   SNode(char name);
   float calcMinArea();
private:
   void calcWandH ();
   bool addToDimensions(Dimensions &nDimension);
};

/***********************************************************************************
 * Constructor: SNode
 * @brief constructs a cell item for a operand
 * @param name the name of the cell
 * @param area the area of the cell
 * @param aspectRatio the aspect ratio of the cell
************************************************************************************/
SNode::SNode(char name, float area, float aspectRatio)
{
   // define the normal data
   this->isOperator = false;
   this->fixed = false;
   this->name = name;
   this->area = area;
   this->aspectRatio = aspectRatio;
   // calculate the size.width and size.height
   calcWandH();
   //wont have a right and left child so will be null
   this->right = NULL;
   this->left = NULL;
   this->parent = NULL;
}

/***********************************************************************************
 * Constructor: SNode
 * @brief constructs a cell item for a operand
 * @param name the name of the cell
 * @param area the area of the cell
 * @param aspectRatio the aspect ratio of the cell
************************************************************************************/
SNode::SNode(char name, float area, float aspectRatio, bool fixed)
{
   // define the normal data
   this->isOperator = false;
   this->fixed = fixed;
   this->name = name;
   this->area = area;
   this->aspectRatio = aspectRatio;
   // calculate the size.width and size.height
   calcWandH();
   //wont have a right and left child so will be null
   this->right = NULL;
   this->left = NULL;
   this->parent = NULL;
}

/***********************************************************************************
 * Constructor: SNode
 * @brief constructs a operator cell 
 * @param name should be a 'V' or 'H' for vertical and horizontal cuts respectively
************************************************************************************/
SNode::SNode(char name)
{
   //define the operator 
   this->isOperator = true;
   this->fixed = true; //operators are always fixed
   this->name = name;
   // default everything else to zero or null
   this->area = 0;
   this->aspectRatio = 0;
   this->right = NULL;
   this->left = NULL;
   this->parent = NULL;
}

/***********************************************************************************
 * Function: calcMinArea
 * @brief gets the area of the cell (or group of cells if it is an operator) also 
 *    defines size.height, size.width, and aspectRatio for operators
 * @return the area of the cell (or group) as a float
************************************************************************************/
float SNode::calcMinArea()
{
   if(isOperator)
   {
      //make sure sizes is currently empty
      sizes.clear();
      // if right or left child is operator calc their values
      if(right->isOperator)
      {
         right->calcMinArea();
      }
      if(left->isOperator)
      {
         left->calcMinArea();
      }
      // if this is a vertical slice do corresponding calculation
      // otherwise do calculation for horizontal slice 
      if (name == 'V')
      {
         for (std::list<Dimensions>::iterator i = right->sizes.begin(); i != right->sizes.end(); i++)
         {
            for (std::list<Dimensions>::iterator j = left->sizes.begin(); j != left->sizes.end(); j++)
            {
               Dimensions nSize;
               nSize.width = i->width + j->width;
               nSize.height = (i->height >= j->height)? i->height : j->height;
               nSize.rSelected = i;
               nSize.lSelected = j;
               addToDimensions(nSize);
            }
         }
      }
      else //it is a horizontal slice
      {
         for (std::list<Dimensions>::iterator i = right->sizes.begin(); i != right->sizes.end(); i++)
         {
            for (std::list<Dimensions>::iterator j = left->sizes.begin(); j != left->sizes.end(); j++)
            {
               Dimensions nSize;
               nSize.width = (i->width >= j->width)? i->width : j->width;
               nSize.height = i->height + j->height;
               nSize.rSelected = i;
               nSize.lSelected = j;
               addToDimensions(nSize);
            }
         }
      }

      //Calculate best area
      std::list<Dimensions>::iterator best = sizes.begin();
      float bestArea = best->height * best->width;
      for(std::list<Dimensions>::iterator current = sizes.begin(); current != sizes.end(); current++)
      {
         float cArea = current->height * current->width;
         if(cArea < bestArea) //if better area found update
         {
            best = current;
            bestArea = cArea;
         }
      }
      area = bestArea;
      selected = *best;
      aspectRatio = selected.height / selected.width;
   }
   return area;
}

/***********************************************************************************
 * Function: calcWandH
 * @brief calculates the size.height and size.width of the cell assigning it to the 
 *    corresponding properties
************************************************************************************/
void SNode::calcWandH()
{
   Dimensions size;
   //calculate normal height and width
   size.height = sqrt(aspectRatio * area);
   size.width = area / size.height;
   sizes.push_back(size);
   //add additional possibilities if not fixed
   if (!fixed)
   {
      float temp = size.height;
      size.height = size.width;
      size.width = temp;
      sizes.push_back(size);
   }
}

/***********************************************************************************
 * Function:addToDimensions
 * @brief adds new dimension to the list after checking that the new value is not
 *    redundant to the list. 
 * @param nDimension the dimension to be added to the list
 * @return true if value was added false if it was not
************************************************************************************/
bool SNode::addToDimensions(Dimensions &nDimension)
{
   //get rid of redundant sizes
   std::list<Dimensions>::iterator item = sizes.begin();
   while (item != sizes.end())
   {
      if(nDimension == *item) // same item already exists so return
      {
         return false;
      }
      else if ((item->height <= nDimension.height) && (item->width <= nDimension.width)) //better item found so return
      {
         return false;
      }
      else if ((item->height >= nDimension.height) && (item->width >= nDimension.width)) //new item better so remove old
      {
         item = sizes.erase(item);
      }
      else
      {
         item++;
      }
   }
   //add on to the back
   sizes.push_back(nDimension);
   return true;
}

/***********************************************************************************
 * Operator: insertion 
 * @brief allows printing the slicing tree in Normalized Polish Expression
 * @param out the output stream to print onto
 * @param rhs the right hand side of the operator
 * @return the same output stream for continued printing
***********************************************************************************/
std::ostream & operator << (std::ostream & out, const SNode & rhs) 
{
   if(rhs.isOperator)
   {
      out << *(rhs.left) << *(rhs.right) <<  rhs.name;
   }
   else
   {
      out << rhs.name;
   }
   return out;
}

/***********************************************************************************
 * Operator: Equivalence
 * @brief equivalence operator for the Dimensions struct
 * @param lhs the left hand side of the operator
 * @param rhs the right hand side of the operator
************************************************************************************/
bool operator== (const Dimensions &lhs, const Dimensions &rhs)
{
   return ((lhs.height == rhs.height) && (lhs.width == rhs.width));
}

#endif