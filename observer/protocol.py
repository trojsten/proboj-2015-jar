
from collections import namedtuple


def file_source(filename):
    with open(filename) as fh:
        for line in fh:
            for number in line.split():
                yield int(number)


def string_source(string):
    return iter(map(int, string.split()))


class _int(object):
    @staticmethod
    def load(source):
        try:
            return next(source)
        except StopIteration:
            raise EOFError()


class vector(object):
    def __init__(self, T):
        self.T = T

    def load(self, source):
        size = _int.load(source)
        return [self.T.load(source) for i in xrange(size)]


class struct(object):
    @classmethod
    def load(cls, source):
        self = cls()
        for fieldtype, fieldname in cls._fields:
            setattr(self, fieldname, fieldtype.load(source))
        return self

    def __repr__(self):
        return '%s(%s)' % (self.__class__.__name__, ', '.join('%s=%r' % (k, getattr(self, k)) for _, k in self._fields))


def load_log(source):
    startovny_teren = Teren.load(source)
    stavy = [Stav.load(source)]
    odpovede = []
    while True:
        try:
            odpovede.append(vector(Odpoved).load(source))
            stavy.append(Stav.load(source))
        except EOFError:
            break
    return startovny_teren, odpovede, stavy


def load_observation(filename):
    result = []
    with open(filename) as fh:
        for line in fh:
            line = line.split()
            result.append(line[0:1] + map(int, line[1:]))
    return result


class Bod(struct):
    _fields = [
        (_int, 'x'),
        (_int, 'y'),
    ]


class Prikaz(struct):
    _fields = [
        (_int, 'kto'),
        (_int, 'typPrikazu'),
        (Bod, 'ciel'),
        (_int, 'parameter'),
    ]

Odpoved = vector(Prikaz)

class Hrac(struct):
    _fields = [
        (_int, 'skore'),
        (vector(_int), 'mapovanie'),
    ]

class Manik(struct):
    _fields = [
        (_int, 'id'),
        (_int, 'x'),
        (_int, 'y'),
        (_int, 'ktorehoHraca'),
        (_int, 'typ'),
        (_int, 'zlato'),
        (_int, 'zelezo'),
        (_int, 'spenat'),
        (_int, 'kovacEnergia'),
    ]

Teren = vector(vector(_int))

class Stav(struct):
    _fields = [
        (vector(Hrac), 'hraci'),
        (vector(Manik), 'manici'),
        (_int, 'dalsiId'),
        (_int, 'cas'),
    ]

class Mapa(struct):
    _fields = [
        (_int, 'pocetHracov'),
        (_int, 'w'),
        (_int, 'h'),
        (Teren, 'pribliznyTeren'),
    ]


def set_format_version(version):
    pass

