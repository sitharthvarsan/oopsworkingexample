Index.html

<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>ResAlloc — Resource Manager</title>
    <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='.9em' font-size='90'>⚡</text></svg>" />
  </head>
  <body>
    <div id="root"></div>
    <script type="module" src="/src/main.jsx"></script>
  </body>
</html>

Main.jsx

import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import App from "./components/App.jsx";

createRoot(document.getElementById("root")).render(
  <StrictMode>
    <App />
  </StrictMode>
);

App.jsx

import { useState, useEffect, useCallback } from "react";
import "../styles/global.css";

import Sidebar          from "./Sidebar.jsx";
import DashboardView    from "./views/DashboardView.jsx";
import UsersView        from "./views/UsersView.jsx";
import ProjectsView     from "./views/ProjectsView.jsx";
import AllocationsView  from "./views/AllocationsView.jsx";
import UserModal        from "./modals/UserModal.jsx";
import ProjectModal     from "./modals/ProjectModal.jsx";
import AllocationModal  from "./modals/AllocationModal.jsx";
import { api }          from "../utils/api.js";

// ─── Toast notification ────────────────────────────────────────────────────────
function Toast({ toasts }) {
  return (
    <div className="toast-wrap">
      {toasts.map((t) => (
        <div key={t.id} className={`toast ${t.type}`}>
          {t.type === "success" ? "✅" : "❌"} {t.msg}
        </div>
      ))}
    </div>
  );
}

export default function App() {
  const [view,        setView]        = useState("dashboard");
  const [users,       setUsers]       = useState([]);
  const [projects,    setProjects]    = useState([]);
  const [allocations, setAllocations] = useState([]);
  const [loading,     setLoading]     = useState(true);
  const [saving,      setSaving]      = useState(false);
  const [toasts,      setToasts]      = useState([]);

  // Modal state: null | "new" | { record }
  const [userModal,  setUserModal]  = useState(null);
  const [projModal,  setProjModal]  = useState(null);
  const [allocModal, setAllocModal] = useState(null);

  // ── Toast helpers ────────────────────────────────────────────────────────────
  const toast = useCallback((msg, type = "success") => {
    const id = Math.random().toString(36).slice(2);
    setToasts((t) => [...t, { id, msg, type }]);
    setTimeout(() => setToasts((t) => t.filter((x) => x.id !== id)), 3500);
  }, []);

  // ── Load all data ────────────────────────────────────────────────────────────
  const loadAll = useCallback(async () => {
    setLoading(true);
    try {
      const [u, p, a] = await Promise.all([
        api.getUsers(),
        api.getProjects(),
        api.getAllocations(),
      ]);
      setUsers(u);
      setProjects(p);
      setAllocations(a);
    } catch (e) {
      toast("Failed to load data. Is the backend running?", "error");
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => { loadAll(); }, []);

  // ── Users CRUD ───────────────────────────────────────────────────────────────
  const saveUser = async (form) => {
    setSaving(true);
    try {
      if (userModal?.id) {
        await api.updateUser(userModal.id, form);
        toast(`${form.name} updated`);
      } else {
        await api.createUser(form);
        toast(`${form.name} added to the team`);
      }
      setUserModal(null);
      const u = await api.getUsers();
      setUsers(u);
    } catch (e) {
      toast(e.message, "error");
    } finally {
      setSaving(false);
    }
  };

  const deleteUser = async (id) => {
    try {
      await api.deleteUser(id);
      toast("Member removed");
      setUsers((u) => u.filter((x) => x.id !== id));
      setAllocations((a) => a.filter((x) => x.user_id !== id));
    } catch (e) {
      toast(e.message, "error");
    }
  };

  // ── Projects CRUD ────────────────────────────────────────────────────────────
  const saveProject = async (form) => {
    setSaving(true);
    try {
      if (projModal?.id) {
        await api.updateProject(projModal.id, form);
        toast(`${form.name} updated`);
      } else {
        const created = await api.createProject(form);
        toast(`Project ${created.code} created`);
      }
      setProjModal(null);
      const p = await api.getProjects();
      setProjects(p);
    } catch (e) {
      toast(e.message, "error");
    } finally {
      setSaving(false);
    }
  };

  const deleteProject = async (id) => {
    try {
      await api.deleteProject(id);
      toast("Project deleted");
      setProjects((p) => p.filter((x) => x.id !== id));
      setAllocations((a) => a.filter((x) => x.project_id !== id));
    } catch (e) {
      toast(e.message, "error");
    }
  };

  // ── Allocations CRUD ─────────────────────────────────────────────────────────
  const saveAllocation = async (form, force = false) => {
    setSaving(true);
    try {
      if (allocModal?.id) {
        await api.updateAllocation(allocModal.id, form, force);
        toast("Allocation updated");
      } else {
        await api.createAllocation(form, force);
        toast("Resource allocated successfully");
      }
      setAllocModal(null);
      const a = await api.getAllocations();
      setAllocations(a);
    } catch (e) {
      if (e.status === 409 && e.conflicts) {
        // Conflict from backend without force — already handled in modal
        toast("Over-allocation conflict — confirm to override", "error");
      } else {
        toast(e.message, "error");
      }
    } finally {
      setSaving(false);
    }
  };

  const deleteAllocation = async (id) => {
    try {
      await api.deleteAllocation(id);
      toast("Allocation removed");
      setAllocations((a) => a.filter((x) => x.id !== id));
    } catch (e) {
      toast(e.message, "error");
    }
  };

  // ── Render ───────────────────────────────────────────────────────────────────
  if (loading) {
    return (
      <div className="loading-center" style={{ minHeight: "100vh", background: "var(--bg)" }}>
        <div className="spinner" />
        <span>Loading ResAlloc…</span>
      </div>
    );
  }

  return (
    <div className="app">
      <Sidebar
        view={view}
        setView={setView}
        counts={{ users: users.length, projects: projects.length, allocations: allocations.length }}
      />

      <main className="main">
        {view === "dashboard" && (
          <DashboardView onNewAlloc={() => setAllocModal("new")} />
        )}
        {view === "users" && (
          <UsersView
            users={users}
            allocations={allocations}
            onNew={() => setUserModal("new")}
            onEdit={(u) => setUserModal(u)}
            onDelete={deleteUser}
          />
        )}
        {view === "projects" && (
          <ProjectsView
            projects={projects}
            allocations={allocations}
            onNew={() => setProjModal("new")}
            onEdit={(p) => setProjModal(p)}
            onDelete={deleteProject}
          />
        )}
        {view === "allocations" && (
          <AllocationsView
            allocations={allocations}
            users={users}
            projects={projects}
            onNew={() => setAllocModal("new")}
            onEdit={(a) => setAllocModal(a)}
            onDelete={deleteAllocation}
          />
        )}
      </main>

      {/* Modals */}
      {userModal && (
        <UserModal
          initial={userModal === "new" ? null : userModal}
          onClose={() => setUserModal(null)}
          onSave={saveUser}
          loading={saving}
        />
      )}
      {projModal && (
        <ProjectModal
          initial={projModal === "new" ? null : projModal}
          onClose={() => setProjModal(null)}
          onSave={saveProject}
          loading={saving}
        />
      )}
      {allocModal && (
        <AllocationModal
          initial={allocModal === "new" ? null : allocModal}
          users={users}
          projects={projects}
          onClose={() => setAllocModal(null)}
          onSave={saveAllocation}
          loading={saving}
        />
      )}

      <Toast toasts={toasts} />
    </div>
  );
}



Sidebar 

export default function Sidebar({ view, setView, counts }) {
  const nav = [
    { id: "dashboard",   label: "Dashboard",     icon: ICONS.dashboard, section: "Overview" },
    { id: "users",       label: "Team Members",  icon: ICONS.users,     section: "Manage", badge: counts.users },
    { id: "projects",    label: "Projects",      icon: ICONS.projects,  section: "Manage", badge: counts.projects },
    { id: "allocations", label: "Allocations",   icon: ICONS.alloc,     section: "Manage", badge: counts.allocations },
  ];

  const sections = [...new Set(nav.map((n) => n.section))];

  return (
    <aside className="sidebar">
      <div className="sidebar-logo">
        <div className="logo-mark">Res<span>Alloc</span></div>
        <div className="logo-sub">Resource Manager</div>
      </div>

      {sections.map((sec) => (
        <div className="nav-section" key={sec}>
          <div className="nav-label">{sec}</div>
          {nav.filter((n) => n.section === sec).map((n) => (
            <div
              key={n.id}
              className={`nav-item ${view === n.id ? "active" : ""}`}
              onClick={() => setView(n.id)}
            >
              <SVG d={n.icon} />
              <span>{n.label}</span>
              {n.badge != null && <span className="nav-badge">{n.badge}</span>}
            </div>
          ))}
        </div>
      ))}

      <div className="sidebar-footer">
        <div className="sf-stat"><span>Team</span><strong>{counts.users} members</strong></div>
        <div className="sf-stat"><span>Projects</span><strong>{counts.projects} total</strong></div>
        <div className="sf-stat"><span>Allocations</span><strong>{counts.allocations} entries</strong></div>
      </div>
    </aside>
  );
}

function SVG({ d, size = 15 }) {
  return (
    <svg width={size} height={size} viewBox="0 0 24 24" fill="none"
      stroke="currentColor" strokeWidth="1.9" strokeLinecap="round" strokeLinejoin="round">
      {typeof d === "string" ? <path d={d} /> : d}
    </svg>
  );
}

const ICONS = {
  dashboard: (
    <>
      <rect x="3" y="3" width="7" height="7" rx="1"/>
      <rect x="14" y="3" width="7" height="7" rx="1"/>
      <rect x="3" y="14" width="7" height="7" rx="1"/>
      <rect x="14" y="14" width="7" height="7" rx="1"/>
    </>
  ),
  users: (
    <>
      <path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/>
      <circle cx="9" cy="7" r="4"/>
      <path d="M23 21v-2a4 4 0 0 0-3-3.87"/>
      <path d="M16 3.13a4 4 0 0 1 0 7.75"/>
    </>
  ),
  projects: (
    <>
      <rect x="2" y="3" width="6" height="6" rx="1"/>
      <path d="M10 6h12"/>
      <rect x="2" y="10" width="6" height="6" rx="1"/>
      <path d="M10 13h12"/>
      <rect x="2" y="17" width="6" height="6" rx="1"/>
      <path d="M10 20h12"/>
    </>
  ),
  alloc: (
    <>
      <circle cx="12" cy="12" r="10"/>
      <polyline points="12 6 12 12 16 14"/>
    </>
  ),
};


Dashboard

import { useState, useEffect } from "react";
import { api } from "../../utils/api.js";
import { fmtShort, weeksFrom, addDays, allocClass, avatarColor, initials, toIso } from "../../utils/helpers.js";

export default function DashboardView({ onNewAlloc }) {
  const [data,       setData]       = useState(null);
  const [loading,    setLoading]    = useState(true);
  const [weekOffset, setWeekOffset] = useState(0);

  const baseDate  = toIso(addDays(new Date(), weekOffset * 8 * 7));
  const weekList  = weeksFrom(new Date(baseDate), 8);

  useEffect(() => {
    setLoading(true);
    api.getDashboard(weekList[0].startIso, 8)
       .then(setData)
       .finally(() => setLoading(false));
  }, [weekOffset]);

  if (loading) return (
    <div className="loading-center">
      <div className="spinner" />
      <span>Loading dashboard…</span>
    </div>
  );

  const { dashboard = [], weeks = [] } = data || {};

  const overCount = dashboard.filter((row) =>
    row.weeks.some((w) => w.total > 100)
  ).length;

  const totalUsers    = dashboard.length;
  const activeAllocs  = dashboard.reduce((s, r) => s + r.weeks.filter(w => w.total > 0).length, 0);

  return (
    <>
      <div className="page-header">
        <div>
          <div className="page-title">Resource Dashboard</div>
          <div className="page-sub">Weekly allocation view across all team members</div>
        </div>
        <button className="btn btn-primary" onClick={onNewAlloc}>
          <PlusIcon /> New Allocation
        </button>
      </div>

      <div className="page-content">
        {/* Stats */}
        <div className="stats-row">
          <StatCard emoji="👥" label="Team Members"   val={totalUsers}  color="var(--accent)" />
          <StatCard emoji="📅" label="Weeks Tracked"  val={weeks.length} color="var(--purple)" />
          <StatCard emoji="✅" label="Active Slots"   val={activeAllocs} color="var(--success)" />
          <StatCard emoji="⚠️" label="Over-Allocated" val={overCount}    color={overCount > 0 ? "var(--danger)" : "var(--success)"} />
        </div>

        {/* Week navigator */}
        <div className="week-nav">
          <button className="btn-icon" onClick={() => setWeekOffset(o => o - 1)}>
            <ChevIcon dir="left" />
          </button>
          <div className="week-range">
            {fmtShort(weekList[0].startIso)} → {fmtShort(weekList[7].endIso)}
          </div>
          <button className="btn-icon" onClick={() => setWeekOffset(o => o + 1)}>
            <ChevIcon dir="right" />
          </button>
          {weekOffset !== 0 && (
            <button className="btn btn-ghost btn-sm" onClick={() => setWeekOffset(0)}>Today</button>
          )}
          <div className="ml-auto flex items-center gap-3 text-sm text-muted" style={{ fontSize: 11 }}>
            <Legend color="var(--danger)"  label="0% (unallocated)" />
            <Legend color="var(--warn)"    label="< 100%" />
            <Legend color="var(--success)" label="= 100%" />
            <Legend color="var(--danger)"  label="> 100% ⚠" />
          </div>
        </div>

        {/* Main table */}
        <div className="dash-wrap">
          <table className="dash-table">
            <thead>
              <tr>
                <th className="dash-th left">Member</th>
                {weeks.map((w, i) => (
                  <th key={i} className="dash-th">
                    W{i + 1}<br />
                    <span style={{ fontSize: 9, fontWeight: 400, color: "var(--text3)" }}>
                      {fmtShort(w.start)}
                    </span>
                  </th>
                ))}
              </tr>
            </thead>
            <tbody>
              {dashboard.length === 0 && (
                <tr>
                  <td colSpan={9} className="dash-td">
                    <div className="empty">
                      <div className="empty-icon">👥</div>
                      <div className="empty-title">No team members</div>
                      <div className="empty-sub">Add members first to see the dashboard.</div>
                    </div>
                  </td>
                </tr>
              )}
              {dashboard.map(({ user, weeks: userWeeks }) => {
                const [ac] = avatarColor(user.name);
                return (
                  <tr key={user.id} className="dash-tr">
                    <td className="dash-td left">
                      <div className="user-cell">
                        <div className="avatar" style={{ background: `${ac}22`, color: ac }}>
                          {initials(user.name)}
                        </div>
                        <div>
                          <div className="u-name">{user.name}</div>
                          <div className="u-role">{user.role}</div>
                        </div>
                      </div>
                    </td>
                    {userWeeks.map((w, wi) => {
                      const pct   = w.total;
                      const cls   = pct === 0 ? "alloc-zero" : allocClass(pct);
                      const title = w.allocations.map(a => `${a.project_code}: ${a.percentage}%`).join("\n");
                      return (
                        <td key={wi} className="dash-td" title={title || "No allocation"}>
                          <span className={`alloc-pill ${cls}`}>{pct}%</span>
                          {w.allocations.length > 1 && (
                            <div style={{ fontSize: 9, color: "var(--text3)", marginTop: 2 }}>
                              {w.allocations.length} proj
                            </div>
                          )}
                        </td>
                      );
                    })}
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      </div>
    </>
  );
}

function StatCard({ emoji, label, val, color }) {
  return (
    <div className="stat-card" style={{ "--c": color }}>
      <div className="stat-emoji">{emoji}</div>
      <div className="stat-val">{val}</div>
      <div className="stat-label">{label}</div>
    </div>
  );
}

function Legend({ color, label }) {
  return (
    <span style={{ display: "inline-flex", alignItems: "center", gap: 4 }}>
      <span style={{ width: 8, height: 8, borderRadius: 2, background: color, display: "inline-block" }} />
      {label}
    </span>
  );
}

function PlusIcon() {
  return (
    <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor"
      strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round">
      <path d="M12 5v14M5 12h14" />
    </svg>
  );
}

function ChevIcon({ dir }) {
  return (
    <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor"
      strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
      <path d={dir === "left" ? "M15 18l-6-6 6-6" : "M9 18l6-6-6-6"} />
    </svg>
  );
}

Userview

import { useState } from "react";
import { avatarColor, initials } from "../../utils/helpers.js";

export default function UsersView({ users, allocations, onNew, onEdit, onDelete }) {
  const [search, setSearch] = useState("");

  const filtered = users.filter(
    (u) =>
      u.name.toLowerCase().includes(search.toLowerCase()) ||
      u.role.toLowerCase().includes(search.toLowerCase()) ||
      (u.department || "").toLowerCase().includes(search.toLowerCase())
  );

  return (
    <>
      <div className="page-header">
        <div>
          <div className="page-title">Team Members</div>
          <div className="page-sub">Manage your team roster and roles</div>
        </div>
        <button className="btn btn-primary" onClick={onNew}>
          <PlusIcon /> Add Member
        </button>
      </div>

      <div className="page-content">
        <div className="mb-4">
          <input
            placeholder="Search by name, role, or department…"
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            style={{ maxWidth: 340 }}
          />
        </div>

        <div className="table-wrap">
          <table>
            <thead>
              <tr>
                <th>Member</th>
                <th>Role</th>
                <th>Department</th>
                <th>Email</th>
                <th>Allocations</th>
                <th style={{ width: 80 }}>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filtered.length === 0 && (
                <tr>
                  <td colSpan={6}>
                    <div className="empty">
                      <div className="empty-icon">👤</div>
                      <div className="empty-title">
                        {search ? "No results found" : "No team members yet"}
                      </div>
                      <div className="empty-sub">
                        {search ? "Try a different search term." : "Click "Add Member" to get started."}
                      </div>
                    </div>
                  </td>
                </tr>
              )}
              {filtered.map((user) => {
                const [ac]       = avatarColor(user.name);
                const userAllocs = allocations.filter((a) => a.user_id === user.id);
                return (
                  <tr key={user.id}>
                    <td>
                      <div className="user-cell">
                        <div className="avatar" style={{ background: `${ac}22`, color: ac }}>
                          {initials(user.name)}
                        </div>
                        <div>
                          <div className="u-name">{user.name}</div>
                        </div>
                      </div>
                    </td>
                    <td>{user.role}</td>
                    <td>
                      <span className="badge badge-gray">{user.department || "—"}</span>
                    </td>
                    <td style={{ fontFamily: "var(--font-mono)", fontSize: 12 }}>{user.email}</td>
                    <td>
                      {userAllocs.length === 0 ? (
                        <span className="text-muted text-sm">None</span>
                      ) : (
                        <span className="badge badge-blue">{userAllocs.length} active</span>
                      )}
                    </td>
                    <td>
                      <div className="flex gap-2">
                        <button className="btn-icon" title="Edit" onClick={() => onEdit(user)}>
                          <EditIcon />
                        </button>
                        <button
                          className="btn-icon danger"
                          title="Delete"
                          onClick={() => {
                            if (window.confirm(`Remove ${user.name} from the team?`))
                              onDelete(user.id);
                          }}
                        >
                          <TrashIcon />
                        </button>
                      </div>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      </div>
    </>
  );
}

function PlusIcon() {
  return <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round"><path d="M12 5v14M5 12h14"/></svg>;
}
function EditIcon() {
  return <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>;
}
function TrashIcon() {
  return <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><polyline points="3 6 5 6 21 6"/><path d="M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6"/><path d="M10 11v6"/><path d="M14 11v6"/><path d="M9 6V4a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2"/></svg>;
}


Projectviews

import { useState } from "react";
import { fmtShort } from "../../utils/helpers.js";

const STATUS_BADGE = {
  active:    "badge-green",
  paused:    "badge-yellow",
  completed: "badge-gray",
  cancelled: "badge-red",
};

export default function ProjectsView({ projects, allocations, onNew, onEdit, onDelete }) {
  const [search, setSearch] = useState("");

  const filtered = projects.filter(
    (p) =>
      p.name.toLowerCase().includes(search.toLowerCase()) ||
      p.client_name.toLowerCase().includes(search.toLowerCase()) ||
      p.code.toLowerCase().includes(search.toLowerCase())
  );

  return (
    <>
      <div className="page-header">
        <div>
          <div className="page-title">Projects</div>
          <div className="page-sub">Manage project metadata and codes</div>
        </div>
        <button className="btn btn-primary" onClick={onNew}>
          <PlusIcon /> New Project
        </button>
      </div>

      <div className="page-content">
        <div className="mb-4">
          <input
            placeholder="Search by project name, client, or code…"
            value={search}
            onChange={(e) => setSearch(e.target.value)}
            style={{ maxWidth: 380 }}
          />
        </div>

        <div className="table-wrap">
          <table>
            <thead>
              <tr>
                <th>Code</th>
                <th>Project Name</th>
                <th>Client</th>
                <th>Dates</th>
                <th>Status</th>
                <th>Resources</th>
                <th style={{ width: 80 }}>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filtered.length === 0 && (
                <tr>
                  <td colSpan={7}>
                    <div className="empty">
                      <div className="empty-icon">📁</div>
                      <div className="empty-title">
                        {search ? "No results found" : "No projects yet"}
                      </div>
                      <div className="empty-sub">
                        {search ? "Try a different search." : "Create a project to start allocating resources."}
                      </div>
                    </div>
                  </td>
                </tr>
              )}
              {filtered.map((proj) => {
                const assigned = allocations.filter((a) => a.project_id === proj.id);
                return (
                  <tr key={proj.id}>
                    <td>
                      <span className="proj-code">{proj.code}</span>
                    </td>
                    <td>
                      <div style={{ fontWeight: 600, color: "var(--text)", fontSize: 13 }}>
                        {proj.name}
                      </div>
                      {proj.description && (
                        <div style={{ fontSize: 11, color: "var(--text3)", marginTop: 2 }}>
                          {proj.description}
                        </div>
                      )}
                    </td>
                    <td style={{ fontWeight: 500 }}>{proj.client_name}</td>
                    <td style={{ fontSize: 12, fontFamily: "var(--font-mono)" }}>
                      {fmtShort(proj.start_date)}
                      <br />
                      <span style={{ color: "var(--text3)" }}>{fmtShort(proj.end_date)}</span>
                    </td>
                    <td>
                      <span className={`badge ${STATUS_BADGE[proj.status] || "badge-gray"}`}>
                        {proj.status}
                      </span>
                    </td>
                    <td>
                      {assigned.length === 0 ? (
                        <span className="text-muted text-sm">Unassigned</span>
                      ) : (
                        <span className="badge badge-purple">{assigned.length} assigned</span>
                      )}
                    </td>
                    <td>
                      <div className="flex gap-2">
                        <button className="btn-icon" title="Edit" onClick={() => onEdit(proj)}>
                          <EditIcon />
                        </button>
                        <button
                          className="btn-icon danger"
                          title="Delete"
                          onClick={() => {
                            if (window.confirm(`Delete project ${proj.code}?`))
                              onDelete(proj.id);
                          }}
                        >
                          <TrashIcon />
                        </button>
                      </div>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      </div>
    </>
  );
}

function PlusIcon() {
  return <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round"><path d="M12 5v14M5 12h14"/></svg>;
}
function EditIcon() {
  return <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>;
}
function TrashIcon() {
  return <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><polyline points="3 6 5 6 21 6"/><path d="M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6"/><path d="M10 11v6"/><path d="M14 11v6"/><path d="M9 6V4a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2"/></svg>;
}



Allocationsviews

import { useState } from "react";
import { fmtDate, avatarColor, initials } from "../../utils/helpers.js";

export default function AllocationsView({ allocations, users, projects, onNew, onEdit, onDelete }) {
  const [filterUser, setFilterUser] = useState("");
  const [filterProj, setFilterProj] = useState("");

  const filtered = allocations.filter((a) => {
    if (filterUser && a.user_id !== filterUser) return false;
    if (filterProj && a.project_id !== filterProj) return false;
    return true;
  });

  return (
    <>
      <div className="page-header">
        <div>
          <div className="page-title">Resource Allocations</div>
          <div className="page-sub">Assign team members to projects with percentages and date ranges</div>
        </div>
        <button className="btn btn-primary" onClick={onNew}>
          <PlusIcon /> New Allocation
        </button>
      </div>

      <div className="page-content">
        {/* Filters */}
        <div className="flex gap-3 mb-4" style={{ flexWrap: "wrap" }}>
          <select value={filterUser} onChange={(e) => setFilterUser(e.target.value)} style={{ width: 200 }}>
            <option value="">All Members</option>
            {users.map((u) => <option key={u.id} value={u.id}>{u.name}</option>)}
          </select>
          <select value={filterProj} onChange={(e) => setFilterProj(e.target.value)} style={{ width: 230 }}>
            <option value="">All Projects</option>
            {projects.map((p) => <option key={p.id} value={p.id}>[{p.code}] {p.name}</option>)}
          </select>
          {(filterUser || filterProj) && (
            <button className="btn btn-ghost btn-sm" onClick={() => { setFilterUser(""); setFilterProj(""); }}>
              Clear filters
            </button>
          )}
        </div>

        <div className="table-wrap">
          <table>
            <thead>
              <tr>
                <th>Member</th>
                <th>Project</th>
                <th>Start Date</th>
                <th>End Date</th>
                <th>Allocation</th>
                <th>Hrs/Day</th>
                <th style={{ width: 80 }}>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filtered.length === 0 && (
                <tr>
                  <td colSpan={7}>
                    <div className="empty">
                      <div className="empty-icon">📅</div>
                      <div className="empty-title">
                        {filterUser || filterProj ? "No matching allocations" : "No allocations yet"}
                      </div>
                      <div className="empty-sub">
                        {filterUser || filterProj
                          ? "Try clearing the filters."
                          : "Assign a team member to a project to begin tracking."}
                      </div>
                    </div>
                  </td>
                </tr>
              )}
              {filtered.map((alloc) => {
                const user = users.find((u) => u.id === alloc.user_id);
                const proj = projects.find((p) => p.id === alloc.project_id);
                const [ac] = user ? avatarColor(user.name) : ["#4f7ef7"];
                const hrs  = (8 * alloc.percentage / 100).toFixed(1);
                const barColor =
                  alloc.percentage > 100 ? "var(--danger)"
                  : alloc.percentage === 100 ? "var(--success)"
                  : "var(--accent)";

                return (
                  <tr key={alloc.id}>
                    <td>
                      {user ? (
                        <div className="user-cell">
                          <div className="avatar" style={{ background: `${ac}22`, color: ac, width: 28, height: 28, fontSize: 11 }}>
                            {initials(user.name)}
                          </div>
                          <span style={{ fontWeight: 500, color: "var(--text)", fontSize: 13 }}>
                            {user.name}
                          </span>
                        </div>
                      ) : <span className="text-muted">Unknown</span>}
                    </td>
                    <td>
                      {proj ? (
                        <div>
                          <span className="proj-code" style={{ marginRight: 6 }}>{proj.code}</span>
                          {proj.name}
                        </div>
                      ) : <span className="text-muted">Unknown</span>}
                    </td>
                    <td className="font-mono text-sm">{fmtDate(alloc.start_date)}</td>
                    <td className="font-mono text-sm">{fmtDate(alloc.end_date)}</td>
                    <td>
                      <div className="flex items-center gap-2">
                        <div className="alloc-bar-wrap">
                          <div className="alloc-bar" style={{ width: `${Math.min(alloc.percentage, 100)}%`, background: barColor }} />
                        </div>
                        <span className="font-bold font-mono" style={{ color: "var(--text)" }}>
                          {alloc.percentage}%
                        </span>
                      </div>
                    </td>
                    <td className="font-mono text-sm">{hrs}h</td>
                    <td>
                      <div className="flex gap-2">
                        <button className="btn-icon" title="Edit" onClick={() => onEdit(alloc)}>
                          <EditIcon />
                        </button>
                        <button
                          className="btn-icon danger"
                          title="Delete"
                          onClick={() => {
                            if (window.confirm("Remove this allocation?")) onDelete(alloc.id);
                          }}
                        >
                          <TrashIcon />
                        </button>
                      </div>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      </div>
    </>
  );
}

function PlusIcon() {
  return <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round"><path d="M12 5v14M5 12h14"/></svg>;
}
function EditIcon() {
  return <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>;
}
function TrashIcon() {
  return <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><polyline points="3 6 5 6 21 6"/><path d="M19 6l-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6"/><path d="M10 11v6"/><path d="M14 11v6"/><path d="M9 6V4a1 1 0 0 1 1-1h4a1 1 0 0 1 1 1v2"/></svg>;
}
