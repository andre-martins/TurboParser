import numpy as np

class Span:
    def __init__(self, start, end, name=''):
        self.start = start
        self.end = end
        self.name = name
        
    def length(self):
        return self.end - self.start + 1
        
    def contains_index(self, i):
        return self.start <= i and self.end >= i
    
    def lies_inside_span(self, span):
        return self.start >= span.start and self.end <= span.end

    def lies_inside_any_of_spans(self, spans):
        for span in spans:
            if self.lies_inside_span(span):
                return True
        return False
        
    def closest_span(self, spans, min_dist=False):
        distances  = []
        for span in spans:
            distances.append(self.distance(span, min_dist))
        span_index = np.argmin(distances)
        return spans[span_index]
        
    def distance(self, span, min_dist=False):
        if min_dist:
            start_interval = min(self.end, span.end)
            end_interval = max(self.start, span.start)
            return max(0, end_interval-start_interval)
        else:
            return abs(self.start-span.start) + abs(self.end-span.end)
        
    def is_equal(self, span):
        return self.start == span.start and self.end == span.end
        
    def overlaps(self, span):
        start_interval = max(self.start, span.start)
        end_interval = min(self.end, span.end)
        return start_interval <= end_interval
        
    def print_span(self):
        print self.name, self.start, self.end
