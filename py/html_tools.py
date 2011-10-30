import xml.etree.ElementTree
import gum

el = xml.etree.ElementTree.Element

def select(ls, name, selected = None, onchange = None):
  select = el("select")
  for s in ls:
    opt = el("option", value = s)
    opt.text = s
    if s == selected: opt.attrib['selected'] = 'selected'
    select.append(opt)
  select.attrib['name'] = name
  if onchange: select.attrib['onchange'] = onchange
  return select

def table(data, **attrib):
  table = el('table', **attrib)
  for rdata in data:
    row = el('tr')
    for cdata in rdata:
      cell = el('td')
      cell.text = str(cdata)
      row.append(cell)
    table.append(row)
  return table
    

def get_set_form(ls, name, **kwargs):
  name_val = name + '_val'
  form = el("form")
  form.attrib['action'] = ""
  fieldset = el("fieldset")

  ls = list(ls)
  ls.sort()

  select_param = {
    'onchange' : 'this.form.' + name_val + '.value=""; this.form.submit();',
  }

  if name_val in kwargs: select_param['selected'] = kwargs[name]

  fieldset.append(select(ls, name, **select_param))
  input_param = {
    'name' : name_val,
  }

  if kwargs.get(name) in ls:
    gumi = gum.Gum()
    if kwargs.get(name_val):
      val = kwargs[name_val]
      if val[0:2] == '0x':
        val = int(val[2:], 0x10)
      else:
        val = int(val)
      gumi.write_symbol(kwargs[name], val)
    input_param['value'] = hex(gumi.read_symbol(kwargs[name]))

  fieldset.append(el('input', type = 'text', **input_param))

  form.append(fieldset)
  return form
