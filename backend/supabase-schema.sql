-- Run this SQL in the Supabase SQL editor for your GDM Loader project.
create table if not exists public.gdm_profiles (
  user_id uuid primary key references auth.users(id) on delete cascade,
  username text not null default '',
  workspace jsonb not null default '{}'::jsonb,
  updated_at timestamptz not null default now()
);
alter table public.gdm_profiles enable row level security;
drop policy if exists "Users can read their own GDM profile" on public.gdm_profiles;
create policy "Users can read their own GDM profile" on public.gdm_profiles
  for select to authenticated using (auth.uid() = user_id);
drop policy if exists "Users can insert their own GDM profile" on public.gdm_profiles;
create policy "Users can insert their own GDM profile" on public.gdm_profiles
  for insert to authenticated with check (auth.uid() = user_id);
drop policy if exists "Users can update their own GDM profile" on public.gdm_profiles;
create policy "Users can update their own GDM profile" on public.gdm_profiles
  for update to authenticated using (auth.uid() = user_id) with check (auth.uid() = user_id);
