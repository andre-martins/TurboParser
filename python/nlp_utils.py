from span import *

def construct_coreference_spans_from_text(span_lines):
    left_bracket = '('
    right_bracket = ')'
    characters_to_ignore = '*-'
    name = ''
    span_names_stack = []
    span_start_stack = []
    spans = []

    for i in xrange(len(span_lines)):
        line = span_lines[i]
        fields = line.split('|')
        for field in fields:
            if field[0] == left_bracket and field[-1] == right_bracket:
                start_position = i
                end_position = i
                name = field[1:-1]
                span = Span(start_position, end_position, name)
                spans.append(span)
            elif field[0] == left_bracket:
                start_position = i
                end_position = -1
                name = field[1:]
                span = Span(start_position, end_position, name)
                spans.append(span)
            elif field[-1] == right_bracket:
                name = field[:-1]
                selected_span = None
                for span in reversed(spans):
                    if span.name == name and span.end == -1:
                        assert selected_span == None, pdb.set_trace()
                        selected_span = span
                        break
                assert selected_span != None, pdb.set_trace()
                selected_span.end = i

    for span in spans:
        assert span.end != -1

    return spans

def construct_coreference_info_from_spans(spans, num_words):
    coreference_span_descriptions = [''] * num_words
    for span in spans:
        if span.start == span.end: # Single-word mention.
            if coreference_span_descriptions[span.start] != '':
                coreference_span_descriptions[span.start] += '|'
            coreference_span_descriptions[span.start] += '(' + span.name + ')'
        else:
            desc = coreference_span_descriptions[span.start]
            if desc != '':
                desc = '|' + desc
            coreference_span_descriptions[span.start] = '(' + span.name + desc
            desc = coreference_span_descriptions[span.end]
            if desc != '':
                desc += '|'
            coreference_span_descriptions[span.end] = desc + span.name + ')'

    for j in xrange(num_words):
        if coreference_span_descriptions[j] == '':
            coreference_span_descriptions[j] = '_'

    return coreference_span_descriptions
