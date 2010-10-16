/*!
 * \file  debug.c
 * \brief  Debugging output functions.
 */

#include <glib/gprintf.h>

#include "debug.h"

/*! \brief  Print debugging info.
 * \param file  Filename of caller.
 * \param func  Function name of caller.
 * \param line  Line number of caller.
 * \param fmt   Format string for a printf.
 */
void debug_out(const char* file,
               const char* func,
               int line,
               const char* fmt,
               ...)
{
  char buf[1024];
  va_list args;

  g_snprintf(buf,1024,"%s(%d) %s: %s\n",file,line,func,fmt);

  va_start(args,fmt);

  g_vfprintf(stderr,buf,args);

  va_end(args);
}

void debug_print_template_tree(GNode* template_tree){
  GNode* template = 0;
  template = g_node_first_child(template_tree);
  
  /* 
   * Loop through all the templates to print
   */
  while(template){
    
    debug_print_field_tree(template, 0);
    template = g_node_next_sibling(template);
  }
}

void debug_print_field_tree (GNode * node, int level){
  
  GNode* child = 0;
  FieldType* field_type = 0;
  field_type = (FieldType*)(node->data);
  debug_print_field (field_type, level*2);
  child = g_node_first_child(node);
  /* Recurse into child nodes */
  while(child){
    debug_print_field_tree (child, level+1);
    child = g_node_next_sibling(child);
  } 
  
}

void debug_print_field (FieldType* field, int indent){
  fprintf(stderr,"%*stype: %s name: %s  operator: %s dictionaryname: %s dictionary_ptr: %p\n",
       indent, "",
       field_typename (field->type),
       field->name,
       operator_typename (field->op),
       field->dictionary,
       (void*)field->dictionary_ptr);
}

