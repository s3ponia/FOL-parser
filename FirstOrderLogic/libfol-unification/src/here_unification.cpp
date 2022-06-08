#include <deque>
#include <libfol-basictypes/term.hpp>
#include <libfol-unification/here_unification.hpp>
#include <map>
#include <unordered_set>
#include <vector>

namespace fol::unification {
namespace {
using SubstitutionMap =
    std::map<types::Variable, std::variant<HEREUnificator::nil, types::Term*,
                                           HEREUnificator::loop>>;
using MapType =
    std::map<types::Variable,
             std::variant<HEREUnificator::nil, types::Term*,
                          HEREUnificator::done, HEREUnificator::loop>>;
using VarSet = std::unordered_set<types::Term*>;
void Here(types::Term& lhs, types::Term& rhs, MapType& map, VarSet& vars);
std::deque<types::Term*> Source(types::Term& r_x, MapType& map);
void Collapse(auto it_beg, auto it_end, types::Term& var, MapType&);
void RecurHere(types::Term& v, types::Term& y, types::Term& t, MapType& map,
               VarSet& vars);
types::Term& Vere(types::Term& var, MapType& map, SubstitutionMap& s);

void VarHere(types::Term& var, types::Term& term, MapType& map, VarSet& vars) {
  vars.insert(&var);
  if (map[var.Var()].index() == HEREUnificator::NIL) {
    map[var.Var()] = &term;
  } else if (!term.IsVar()) {
    auto path = Source(var, map);
    Collapse(path.begin() + 1, path.end(), *path.front(), map);
    if (map[path.front()->Var()].index() == HEREUnificator::NIL) {
      map[path.front()->Var()] = &term;
    } else {
      RecurHere(*path.front(),
                *std::get<HEREUnificator::TERM>(map[path.front()->Var()]), term,
                map, vars);
    }
  } else {
    if (map[var.Var()].index() == HEREUnificator::NIL) {
      map[term.Var()] = &var;
    } else {
      auto path = Source(var, map);
      auto* v = path.front();
      path.pop_front();
      if (map[v->Var()].index() == HEREUnificator::NIL ||
          map[v->Var()].index() == HEREUnificator::DONE) {
        path.push_front(v);
        Collapse(path.begin(), path.end(), term, map);
        path.pop_front();
      } else {
        auto path2 = Source(term, map);
        auto* w = path2.front();
        path2.pop_front();

        using types::operator==;
        if (*v == *w) {
          Collapse(path.begin(), path.end(), *v, map);
          Collapse(path2.begin(), path2.end(), *v, map);
        } else {
          auto z = map[w->Var()];
          map[w->Var()] = v;
          Collapse(path.begin(), path.end(), *v, map);
          Collapse(path2.begin(), path2.end(), *v, map);
          if (z.index() != HEREUnificator::NIL &&
              z.index() != HEREUnificator::DONE) {
            RecurHere(*v, *std::get<HEREUnificator::TERM>(map[v->Var()]),
                      *std::get<HEREUnificator::TERM>(z), map, vars);
          }
        }
      }
    }
  }
}

void RecurHere(types::Term& v, types::Term& y, types::Term& t, MapType& map,
               VarSet& vars) {
  map[v.Var()] = HEREUnificator::loop{};
  Here(y, t, map, vars);
  map[v.Var()] = &y;
}

void Here(types::Term& lhs, types::Term& rhs, MapType& map, VarSet& vars) {
  using types::operator==;
  if (lhs != rhs && !lhs.IsConstant() && !rhs.IsConstant()) {
    if (lhs.IsVar()) {
      VarHere(lhs, rhs, map, vars);
    } else if (rhs.IsVar()) {
      VarHere(rhs, lhs, map, vars);
    } else {
      auto& f_lhs = lhs.Function();
      auto& f_rhs = rhs.Function();

      if (FunctionName(f_lhs) != FunctionName(f_rhs)) {
        throw HEREUnificator::ExitClash{"CLASH"};
      } else {
        auto lhs_t_list = FunctionTermsIt(lhs.Function());
        auto rhs_t_list = FunctionTermsIt(rhs.Function());

        for (; lhs_t_list != parser::TermListIt{} &&
               rhs_t_list != parser::TermListIt{};
             ++lhs_t_list, ++rhs_t_list) {
          Here(*lhs_t_list, *rhs_t_list, map, vars);
        }
      }
    }
  }
}

std::deque<types::Term*> Source(types::Term& r_x, MapType& map) {
  std::deque<types::Term*> path;
  auto* x = &r_x;
  path.push_back(x);

  types::Term* z;
  while (map[x->Var()].index() == HEREUnificator::TERM &&
         std::get<HEREUnificator::TERM>(map[x->Var()])->IsVar()) {
    z = std::get<HEREUnificator::TERM>(map[x->Var()]);
    path.push_front(z);
    map[x->Var()] = HEREUnificator::done{};
    x = z;
  }

  if (x == &r_x && path.size() > 1) {
    path.pop_front();
  }

  return path;
}

void Collapse(auto it_beg, auto it_end, types::Term& var, MapType& map) {
  std::for_each(it_beg, it_end, [&](auto&& item) { map[item->Var()] = &var; });
}

types::Term& TermVere(parser::Term& t, MapType& map, SubstitutionMap& s) {
  auto t_list = FunctionTermsIt(t.Function());
  for (; t_list != parser::TermListIt{}; ++t_list) {
    if (t_list->IsVar()) {
      if (map[t_list->Var()].index() == HEREUnificator::DONE) {
        if (s[t_list->Var()].index() == HEREUnificator::LOOP) {
          throw HEREUnificator::ExitLoop{"LOOP"};
        } else {
          *t_list =
              types::Clone(*std::get<HEREUnificator::TERM>(s[t_list->Var()]));
        }
      } else if (map[t_list->Var()].index() != HEREUnificator::NIL) {
        *t_list = types::Clone(Vere(*t_list, map, s));
      }
    } else if (t_list->IsFunction()) {
      *t_list = types::Clone(TermVere(*t_list, map, s));
    }
  }

  return t;
}

types::Term& Vere(types::Term& var, MapType& map, SubstitutionMap& s) {
  auto path = Source(var, map);
  if (path.empty()) {
    return var;
  }

  auto* v = path.front();
  path.pop_front();

  types::Term* z;

  if (map[v->Var()].index() == HEREUnificator::NIL) {
    z = v;
    map[v->Var()] = HEREUnificator::done{};
  } else if (map[v->Var()].index() == HEREUnificator::TERM &&
             std::get<HEREUnificator::TERM>(map[v->Var()])->IsConstant()) {
    z = std::get<HEREUnificator::TERM>(map[v->Var()]);
    s[v->Var()] = z;
    map[v->Var()] = HEREUnificator::done{};
  } else if (map[v->Var()].index() == HEREUnificator::TERM &&
             std::get<HEREUnificator::TERM>(map[v->Var()])->IsFunction()) {
    auto* y = std::get<HEREUnificator::TERM>(map[v->Var()]);
    map[v->Var()] = HEREUnificator::done{};
    s[v->Var()] = HEREUnificator::loop{};
    for (auto& x : path) {
      s[x->Var()] = HEREUnificator::loop{};
    }
    z = &TermVere(*y, map, s);
    s[v->Var()] = z;
  } else {
    if (s[v->Var()].index() == HEREUnificator::NIL) {
      z = v;
    } else if (s[v->Var()].index() == HEREUnificator::LOOP) {
      throw HEREUnificator::ExitLoop{"LOOP"};
    } else {
      z = std::get<HEREUnificator::TERM>(s[v->Var()]);
    }
  }

  for (auto& x : path) {
    s[x->Var()] = z;
  }

  return *z;
}

Substitution SubstitutionFromMap(MapType& map, VarSet& vars) {
  SubstitutionMap s;

  for (auto& v : vars) {
    Vere(*v, map, s);
  }
  std::vector<Substitution::SubstitutePair> pairs;
  pairs.reserve(s.size());
  for (auto& [k, v] : s) {
    if (v.index() == HEREUnificator::TERM) {
      pairs.emplace_back(k, types::Clone(*std::get<HEREUnificator::TERM>(v)));
    }
  }

  return Substitution{std::move(pairs)};
}
}  // namespace

std::optional<Substitution> HEREUnificator::Unificate(
    const types::Atom& lhs, const types::Atom& rhs) const try {
  if (lhs.predicate_name() != rhs.predicate_name() ||
      lhs.terms_size() != rhs.terms_size()) {
    return std::nullopt;
  }

  auto cp_lhs = lhs;
  auto cp_rhs = rhs;

  MapType map;
  VarSet vars;

  for (std::size_t i = 0; i < lhs.terms_size(); ++i) {
    Here(cp_lhs[i], cp_rhs[i], map, vars);
  }

  return SubstitutionFromMap(map, vars);
} catch (const ExitLoop& ex) {
  return std::nullopt;
} catch (const ExitClash& ex) {
  return std::nullopt;
}
}  // namespace fol::unification
