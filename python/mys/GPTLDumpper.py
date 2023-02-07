### Test
# dumper = GPTLDumpper("batch.log/gcr.log.211210#1/run.50km.step71.32.8.256#1/gptl_output/timing000000.txt")
# dumper.dumpjson("err")
# print(dumper.topandas())

class GPTLDumpper:
    def __init__(self, filename):
        self.filename = filename
        with open(filename) as f:
            lines = f.read().splitlines()
        self.events = []
        self.eventNum = 0
        self.header = None
        self.stats = []
        self.statNum = 0

        phase = "Begin"
        for i in range(len(lines)):
            line = lines[i]
            if phase == "Begin" and "PAPI events enabled" in line:
                # event name start
                phase = "DumpEventNameStart"
            elif phase == "Begin" and "Stats for thread 0" in line:
                phase = "DumpHeaderStart"
            elif phase == "DumpEventNameStart":
                # Get event name
                eventName = line.strip()
                if len(eventName) == 0: 
                    phase = "DumpEventNameEnd"
                    continue
                self.events.append(eventName)
                self.eventNum += 1
            elif phase == "DumpEventNameEnd" and "Stats for thread 0" in line:
                phase = "DumpHeaderStart"
            elif phase == "DumpHeaderStart":
                # Get header
                self.header = line.strip().split()
                if self.eventNum > 0:
                    self.header[-len(self.events):] = self.events
                statNameEndIndex = line.index(self.header[0])
                phase = "DumpStatStart"
            elif phase == "DumpStatStart":
                # Get stats
                cells = line.strip().split()
                if len(cells) == 0:
                    phase = "DumpStatEnd"
                    continue
                headerLen = len(self.header)
                vals = cells[-headerLen:]
                statName = line[:statNameEndIndex].strip()
                if statName.startswith("*"):
                    statName = statName[1:].strip()
                res = {"name": statName}
                for i in range(headerLen):
                    h, v = self.header[i], vals[i]
                    ### Some values that we want integer rather than float
                    preferIntegerMatchers = [
                                    self.header[i] == "Called",          # Called
                                    self.header[i] == "AVG_MPI_BYTES",   # AVG_MPI_BYTES if possible
                                    not(i < headerLen-self.eventNum),    # PMU events
                    ]
                    res[h] = GPTLDumpper.tryConvertToNumber(v, any(preferIntegerMatchers) == True)
                self.stats.append(res)
                self.statNum += 1

    def get(self, name):
        for stat in self.stats:
            if stat["name"] == name:
                return stat

    def dumpjson(self, filename):
        import json
        with open(filename, "w") as fp:
            json.dump(self.stats, fp, indent=4)

    def topandas(self):
        import pandas as pd
        return pd.DataFrame(self.stats)

    @staticmethod
    def tryConvertToNumber(text, preferInteger=True):
        if preferInteger:
            try:
                return int(float(text))
            except:
                try:
                    return float(text)
                except:
                    return text
        else:
            try:
                return float(text)
            except:
                return text
