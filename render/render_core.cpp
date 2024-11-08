static R_Handle
r_handle_zero()
{
 R_Handle result = {};
 return result;
}

static B32
r_handle_match(R_Handle a, R_Handle b)
{
 B32 result = a.u64[0] == b.u64[0];
 return result;
}

static void
r_add_input_layout_attribute(R_InputLayoutDesc *input_layout_desc, R_AttributeDesc desc)
{
 if(input_layout_desc->attribs_count < array_count(input_layout_desc->attribs))
 {
  R_InputLayoutAttribute *attrib = &input_layout_desc->attribs[input_layout_desc->attribs_count];
  attrib->name = desc.semantic_name;
  attrib->kind = desc.kind;
  attrib->offset = desc.offset;
  attrib->slot_class = desc.slot_class;
  attrib->semantic_index = desc.semantic_index;
  attrib->step_rate = desc.step_rate;
  input_layout_desc->attribs_count++;
 }
}