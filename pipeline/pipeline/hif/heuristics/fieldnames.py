import pipeline.infrastructure.api as api

class IntentFieldnames(api.Heuristic):
    def calculate(self, ms, intent):
        fields = ms.get_fields(intent=intent)        
        names = set([f.name for f in fields])
        
        # return names if no identically-named field exists with other
        # intents, otherwise return fields IDs. 
        fields_with_name = ms.get_fields(name = names)
        if len(fields_with_name) != len(fields):
            return ','.join([str(f.id) for f in fields])
        else:
            return ','.join(names)
