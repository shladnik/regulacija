import xml.etree.ElementTree

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
